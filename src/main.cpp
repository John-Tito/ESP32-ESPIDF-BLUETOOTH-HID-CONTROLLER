/* This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this software is
   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "hal/timer_hal.h"
#include "sdkconfig.h"
#include <iostream>
#include <string.h>

#include "controlerHandler.hpp"
#include "esp_hid_gap.h"
#include "lvgl.h"
#include "lvgl_helpers.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void inpu_dev_init(void);
static void btn_test(void);
static void xInput_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
void task_ui(void *param);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_group_t *g;
static const char *TAG_UI = "ui";
static const char *TAG_INIT = "init";

extern "C" void app_main(void)
{
    if (!controler_hid_init())
        ESP_LOGI(TAG_INIT, "err when init hid");
    xTaskCreate(&task_ui, "ui_loop", 10 * 1024, NULL, 2, NULL);
}

/* create frame buffer */
SemaphoreHandle_t xGUISemaphore;
void task_ui(void *param)
{
    xGUISemaphore = xSemaphoreCreateMutex();
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t *frame_buf1;
    static lv_color_t *frame_buf2;

    lvgl_driver_init();
    ESP_LOGI(TAG_UI, "lvgl_driver_init_done");

    lv_init();
    ESP_LOGI(TAG_UI, "lv_init_done");

    // #pragma region draw_buf_init
    frame_buf1 = (lv_color_t *)heap_caps_malloc((DISP_BUF_SIZE * (LV_COLOR_DEPTH / 8)), MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT);
    frame_buf2 = (lv_color_t *)heap_caps_malloc((DISP_BUF_SIZE * (LV_COLOR_DEPTH / 8)), MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT);
    if (NULL == frame_buf1)
    {
        frame_buf1 = (lv_color_t *)heap_caps_malloc((DISP_BUF_SIZE * (LV_COLOR_DEPTH / 8)), MALLOC_CAP_DEFAULT | MALLOC_CAP_32BIT);
        ESP_LOGW(TAG_UI, "err when malloc frame buffer1 from psram");
    }
    if (NULL == frame_buf2)
    {
        frame_buf2 = (lv_color_t *)heap_caps_malloc((DISP_BUF_SIZE * (LV_COLOR_DEPTH / 8)), MALLOC_CAP_DEFAULT | MALLOC_CAP_32BIT);
        ESP_LOGW(TAG_UI, "err when malloc frame buffer2 from psram");
    }

    if ((NULL == frame_buf1) || (NULL == frame_buf2))
    {
        ESP_LOGE(TAG_UI, "err when malloc frame buffer");
        exit(-4);
    }
    lv_disp_draw_buf_init(&draw_buf, frame_buf1, frame_buf2, DISP_BUF_SIZE);
    // #pragma endregion

    // #pragma region display_driver_init
    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = CONFIG_LV_HOR_RES_MAX;
    disp_drv.ver_res = CONFIG_LV_VER_RES_MAX;
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    // #pragma endregion

    // #pragma region input_driver_init
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*See below.*/
    indev_drv.read_cb = xInput_read;        /*See below.*/
    lv_indev_drv_register(&indev_drv);
    ESP_LOGI(TAG_UI, "lv_disp_drv_init_done");
    inpu_dev_init();
    // #pragma endregion

    btn_test();

    while (1)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        lv_task_handler();
    }
    vTaskDelete(NULL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void mouse_get_xy(int16_t *x, int16_t *y)
{
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    int32_t inc_x = 0, inc_y = 0;

    inc_x = (10 * ((int32_t)xInputParser.joyLHori - (int32_t)xInputParser.joyMid) / (int32_t)xInputParser.joyMax);
    inc_y = (10 * ((int32_t)xInputParser.joyLVert - (int32_t)xInputParser.joyMid) / (int32_t)xInputParser.joyMax);

    last_x += inc_x;
    last_x = (last_x < CONFIG_LV_HOR_RES_MAX) ? last_x : CONFIG_LV_HOR_RES_MAX;
    last_x = (last_x > 0) ? last_x : 0;
    *x = last_x;

    last_y += inc_y;
    last_y = (last_y < CONFIG_LV_VER_RES_MAX) ? last_y : CONFIG_LV_VER_RES_MAX;
    last_y = (last_y > 0) ? last_y : 0;
    *y = last_y;
}

static lv_key_t keypad_get_key(void)
{
    lv_key_t key = 0;

    if (xInputParser.btnDirUp)
    {
        key = LV_KEY_UP;
    }
    else if (xInputParser.btnDirDown)
    {
        key = LV_KEY_DOWN;
    }
    else if (xInputParser.btnDirLeft)
    {
        key = LV_KEY_LEFT;
    }
    else if (xInputParser.btnDirRight)
    {
        key = LV_KEY_RIGHT;
    }
    else if (xInputParser.btnA)
    {
        key = LV_KEY_ENTER;
    }
    else if (xInputParser.btnB)
    {
        key = LV_KEY_ESC;
    }
    else if (xInputParser.btnLB)
    {
        key = LV_KEY_PREV;
    }
    else if (xInputParser.btnRB)
    {
        key = LV_KEY_NEXT;
    }
    return key;
}

static void xInput_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{

    static long last_time = 0;

    if (pdTRUE == xSemaphoreTake(xInputSemaphore, 0))
    {
        if (xInputParser.outOfDate)
        {
            mouse_get_xy(&data->point.x, &data->point.y);

            if (xInputParser.btnA | xInputParser.btnB | xInputParser.btnX | xInputParser.btnY)
            {
                data->state = LV_INDEV_STATE_PR;
                last_time = (long)esp_timer_get_time() / 1000;
                ESP_LOGI(TAG_UI, "down:%x", xInputRawData[14]);
            }
            else
            {
                data->state = LV_INDEV_STATE_REL;
            }
            xInputParser.outOfDate = false;
        }
        else
        {
            if (data->state == LV_INDEV_STATE_PR)
            {
                if ((((long)esp_timer_get_time() / 1000) - last_time) > 60)
                {
                    data->state = LV_INDEV_STATE_REL;
                    ESP_LOGI(TAG_UI, "up");
                }
            }
        }
        xSemaphoreGive(xInputSemaphore);
    }
}

static void inpu_dev_init()
{
    g = lv_group_create();
    lv_group_set_default(g);

    lv_indev_t *cur_drv = NULL;
    for (;;)
    {
        cur_drv = lv_indev_get_next(cur_drv);
        if (!cur_drv)
        {
            break;
        }

        if (cur_drv->driver->type == LV_INDEV_TYPE_KEYPAD)
        {
            lv_indev_set_group(cur_drv, g);
        }

        if (cur_drv->driver->type == LV_INDEV_TYPE_ENCODER)
        {
            lv_indev_set_group(cur_drv, g);
        }

        if (cur_drv->driver->type == LV_INDEV_TYPE_POINTER)
        {
            lv_obj_t *cursor_img = lv_img_create(lv_scr_act());
            lv_img_set_src(cursor_img, LV_SYMBOL_OK);
            lv_indev_set_cursor(cur_drv, cursor_img);
        }
    }
}

static void event_handler(lv_event_t *event)
{
    if (LV_EVENT_CLICKED == event->code)
    {
        ESP_LOGI(TAG_UI, "Clicked\n");
    }
    else if (LV_EVENT_VALUE_CHANGED == event->code)
    {
        ESP_LOGI(TAG_UI, "Toggled\n");
    }
}

static void btn_test()
{
    lv_obj_t *label;
    lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, 40);
    label = lv_label_create(btn1);
    lv_label_set_text(label, "A");

    lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, -40);
    label = lv_label_create(btn2);
    lv_label_set_text(label, "B");
}