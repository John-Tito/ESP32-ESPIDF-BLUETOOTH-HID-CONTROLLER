/* This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this software is
   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include <iostream>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "hal/timer_hal.h"

#include "XboxControllerNotificationParser.hpp" // This will pull in our MPG implementation
#include "controlerHandler.hpp"
#include "esp_hid_gap.h"
#include "lvgl.h"
#include "lvgl_helpers.h"

static const char *TAG = "TEST";
XboxControllerNotificationParser parser;
uint8_t raw1[17];
XboxController_input_report_t raw2;

void lv_ex_list_1(void);
void task_printout(void *param);
void task_ui(void *param);
void lv_update_request(void *);

extern "C" void app_main(void)
{
    if (controler_hid_init())
        xTaskCreate(&task_printout, "key_out", 10 * 1024, NULL, 2, NULL);
    xTaskCreate(&task_ui, "ui_loop", 10 * 1024, NULL, 2, NULL);
}

void task_printout(void *param)
{
    while (NULL == controllerQueue)
    {
        vTaskDelay(1000);
        ESP_LOGI(TAG, "wait queue...");
    }
    while (1)
    {
        if (xQueueReceive(controllerQueue, &raw1, (TickType_t)10))
            if (0 == parser.update(&raw1[1], raw1[0]))
            {
                ESP_LOGI(TAG, "LT:%d", parser.trigLT);
                memcpy(&raw2, &raw1[1], raw1[0]);
                ESP_LOGI(TAG, "LX:%d", raw2.joyLX);
            }
        ESP_LOG_BUFFER_HEX(TAG, (void *)&raw1[1], raw1[0]);
        vTaskDelay(50);
    }
    vTaskDelete(NULL);
}
// // void (*_lv_indev_drv_t::read_cb)(_lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
// void xInput_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
// {
//     data->point.x = 0;
//     data->point.y = 0;
//     data->state = LV_INDEV_STATE_PRESSED;

//     data->key =
// }

/* create frame buffer */
SemaphoreHandle_t xGUISemaphore;
void task_ui(void *param)
{
    xGUISemaphore = xSemaphoreCreateMutex();
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t *frame_buf1;
    static lv_color_t *frame_buf2;

    frame_buf1 = (lv_color_t *)heap_caps_malloc((DISP_BUF_SIZE * (LV_COLOR_DEPTH / 8)), MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT);
    frame_buf2 = (lv_color_t *)heap_caps_malloc((DISP_BUF_SIZE * (LV_COLOR_DEPTH / 8)), MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT);
    if (NULL == frame_buf1)
    {
        frame_buf1 = (lv_color_t *)heap_caps_malloc((DISP_BUF_SIZE * (LV_COLOR_DEPTH / 8)), MALLOC_CAP_DEFAULT | MALLOC_CAP_32BIT);
        ESP_LOGW("ui", "err when malloc frame buffer1 from psram");
    }
    if (NULL == frame_buf2)
    {
        frame_buf2 = (lv_color_t *)heap_caps_malloc((DISP_BUF_SIZE * (LV_COLOR_DEPTH / 8)), MALLOC_CAP_DEFAULT | MALLOC_CAP_32BIT);
        ESP_LOGW("ui", "err when malloc frame buffer2 from psram");
    }

    if ((NULL == frame_buf1) || (NULL == frame_buf2))
    {
        ESP_LOGE("ui", "err when malloc frame buffer");
        exit(-4);
    }

    lvgl_driver_init();
    ESP_LOGI("ui", "lvgl_driver_init_done");

    lv_init();
    ESP_LOGI("ui", "lv_init_done");

    lv_disp_draw_buf_init(&draw_buf, frame_buf1, frame_buf2, DISP_BUF_SIZE);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    /*Change the following line to your display resolution*/
    disp_drv.hor_res = CONFIG_LV_VER_RES_MAX;
    disp_drv.ver_res = CONFIG_LV_HOR_RES_MAX;
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/

    // indev_drv.type = LV_INDEV_TYPE_KEYPAD; /*See below.*/
    // indev_drv.read_cb = xInput_read;       /*See below.*/

    // /*Register the driver in LVGL and save the created input device object*/
    // lv_indev_t *my_indev = lv_indev_drv_register(&indev_drv);

    ESP_LOGI("ui", "lv_disp_drv_init_done");

    // esp_timer_create_args_t ui_timer_args = {
    //     .callback = lv_update_request,
    //     .name = "ui_timer"};
    // esp_timer_handle_t ui_timer;

    // ESP_ERROR_CHECK(esp_timer_create(&ui_timer_args, &ui_timer));
    // ESP_ERROR_CHECK(esp_timer_start_periodic(ui_timer, CONFIG_LV_DISP_DEF_REFR_PERIOD * 1000));

    lv_ex_list_1();

    while (1)
    {

        vTaskDelay(30 / portTICK_PERIOD_MS);
        // xSemaphoreTake(xGUISemaphore, portMAX_DELAY);
        lv_timer_handler();
        // xSemaphoreGive(xGUISemaphore);
    }
    vTaskDelete(NULL);
}

void lv_ex_list_1(void)
{
    /*Create a list*/
    lv_obj_t *list1 = lv_list_create(lv_scr_act());
    lv_obj_set_size(list1, 128, 128);
    lv_obj_align(list1, LV_ALIGN_CENTER, 0, 0);

    /*Add buttons to the list*/
    lv_obj_t *list_btn;

    list_btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, "New");
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, "Open");
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Delete");
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_EDIT, "Edit");
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_SAVE, "Save");
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_BELL, "Notify");
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_BATTERY_FULL, "Battery");
}

void lv_update_request(void *)
{
    lv_tick_inc(CONFIG_LV_DISP_DEF_REFR_PERIOD);
}