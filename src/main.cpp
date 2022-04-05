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
// (long)esp_timer_get_time()/1000
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void inpu_dev_init(void);
static void selectors_create(lv_obj_t *parent);
static void text_input_create(lv_obj_t *parent);
static void msgbox_create(void);
static void lv_ex_list_1(void);

void task_ui(void *param);

static void msgbox_event_cb(lv_event_t *e);
static void ta_event_cb(lv_event_t *e);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_group_t *g;
static lv_obj_t *tv;
static lv_obj_t *t1;
static lv_obj_t *t2;
static const char *TAG_UI = "ui";
static const char *TAG_INIT = "init";

extern "C" void app_main(void)
{
    if (!controler_hid_init())
        ESP_LOGI(TAG_INIT, "err when init hid");
    xTaskCreate(&task_ui, "ui_loop", 10 * 1024, NULL, 2, NULL);
}

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

static uint32_t keypad_get_key(void)
{
    static uint32_t key = 0;

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
        key = LV_KEY_PREV;
    }
    else if (xInputParser.btnX)
    {
        key = LV_KEY_NEXT;
    }
    else if (xInputParser.btnY)
    {
        key = LV_KEY_ESC;
    }
    // ESP_LOGI(TAG_UI, "k:%d", key);
    return key;
}

static void xInput_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{

    static uint32_t last_key = 0;
    uint32_t act_key = 0;
    if (pdTRUE == xSemaphoreTake(xInputSemaphore, 0))
    {
        if (xInputParser.outOfDate)
        {
            xInputParser.outOfDate = false;
            act_key = keypad_get_key();
            mouse_get_xy(&data->point.x, &data->point.y);
        }
        xSemaphoreGive(xInputSemaphore);
    }

    if (act_key == LV_KEY_ENTER)
    {
        data->state = LV_INDEV_STATE_PR;
        last_key = act_key;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
    data->key = last_key;
}

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

    lvgl_driver_init();
    ESP_LOGI(TAG_UI, "lvgl_driver_init_done");

    lv_init();
    ESP_LOGI(TAG_UI, "lv_init_done");

    lv_disp_draw_buf_init(&draw_buf, frame_buf1, frame_buf2, DISP_BUF_SIZE);
    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    /*Change the following line to your display resolution*/
    disp_drv.hor_res = CONFIG_LV_HOR_RES_MAX;
    disp_drv.ver_res = CONFIG_LV_VER_RES_MAX;
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    static lv_indev_t *indev = NULL;

    // lv_indev_drv_init(&indev_drv);         /*Basic initialization*/
    // indev_drv.type = LV_INDEV_TYPE_KEYPAD; /*See below.*/
    // indev_drv.read_cb = xInput_read;       /*See below.*/
    // indev = lv_indev_drv_register(&indev_drv);

    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*See below.*/
    indev_drv.read_cb = xInput_read;        /*See below.*/
    lv_indev_drv_register(&indev_drv);

    ESP_LOGI(TAG_UI, "lv_disp_drv_init_done");

    // esp_timer_create_args_t ui_timer_args = {
    //     .callback = lv_update_request,
    //     .name = "ui_timer"};
    // esp_timer_handle_t ui_timer;

    // ESP_ERROR_CHECK(esp_timer_create(&ui_timer_args, &ui_timer));
    // ESP_ERROR_CHECK(esp_timer_start_periodic(ui_timer, CONFIG_LV_DISP_DEF_REFR_PERIOD * 1000));

    inpu_dev_init();

    while (1)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        lv_task_handler();
    }
    vTaskDelete(NULL);
}

static void lv_ex_list_1(void)
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

    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, LV_DPI_DEF / 3);

    t1 = lv_tabview_add_tab(tv, "Selectors");
    t2 = lv_tabview_add_tab(tv, "Text input");

    selectors_create(t1);
    text_input_create(t2);

    msgbox_create();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void selectors_create(lv_obj_t *parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *obj;

    obj = lv_table_create(parent);
    lv_table_set_cell_value(obj, 0, 0, "00");
    lv_table_set_cell_value(obj, 0, 1, "01");
    lv_table_set_cell_value(obj, 1, 0, "10");
    lv_table_set_cell_value(obj, 1, 1, "11");
    lv_table_set_cell_value(obj, 2, 0, "20");
    lv_table_set_cell_value(obj, 2, 1, "21");
    lv_table_set_cell_value(obj, 3, 0, "30");
    lv_table_set_cell_value(obj, 3, 1, "31");
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_calendar_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_btnmatrix_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_checkbox_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_slider_create(parent);
    lv_slider_set_range(obj, 0, 10);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_switch_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_spinbox_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_dropdown_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_roller_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_t *list = lv_list_create(parent);
    lv_obj_update_layout(list);
    if (lv_obj_get_height(list) > lv_obj_get_content_height(parent))
    {
        lv_obj_set_height(list, lv_obj_get_content_height(parent));
    }

    lv_list_add_btn(list, LV_SYMBOL_OK, "Apply");
    lv_list_add_btn(list, LV_SYMBOL_CLOSE, "Close");
    lv_list_add_btn(list, LV_SYMBOL_EYE_OPEN, "Show");
    lv_list_add_btn(list, LV_SYMBOL_EYE_CLOSE, "Hide");
    lv_list_add_btn(list, LV_SYMBOL_TRASH, "Delete");
    lv_list_add_btn(list, LV_SYMBOL_COPY, "Copy");
    lv_list_add_btn(list, LV_SYMBOL_PASTE, "Paste");
}

static void text_input_create(lv_obj_t *parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *ta1 = lv_textarea_create(parent);
    lv_obj_set_width(ta1, LV_PCT(100));
    lv_textarea_set_one_line(ta1, true);
    lv_textarea_set_placeholder_text(ta1, "Click with an encoder to show a keyboard");

    lv_obj_t *ta2 = lv_textarea_create(parent);
    lv_obj_set_width(ta2, LV_PCT(100));
    lv_textarea_set_one_line(ta2, true);
    lv_textarea_set_placeholder_text(ta2, "Type something");

    lv_obj_t *kb = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(ta1, ta_event_cb, LV_EVENT_ALL, kb);
    lv_obj_add_event_cb(ta2, ta_event_cb, LV_EVENT_ALL, kb);
}

static void msgbox_create(void)
{
    static const char *btns[] = {"Ok", "Cancel", ""};
    lv_obj_t *mbox = lv_msgbox_create(NULL, "Hi", "Welcome to the keyboard and encoder demo", btns, false);
    lv_obj_add_event_cb(mbox, msgbox_event_cb, LV_EVENT_ALL, NULL);
    lv_group_focus_obj(lv_msgbox_get_btns(mbox));
    lv_obj_add_state(lv_msgbox_get_btns(mbox), LV_STATE_FOCUS_KEY);
#if LV_EX_MOUSEWHEEL
    lv_group_set_editing(g, true);
#endif
    lv_group_focus_freeze(g, true);

    lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *bg = lv_obj_get_parent(mbox);
    lv_obj_set_style_bg_opa(bg, LV_OPA_70, 0);
    lv_obj_set_style_bg_color(bg, lv_palette_main(LV_PALETTE_GREY), 0);
}

static void msgbox_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        const char *txt = lv_msgbox_get_active_btn_text(msgbox);
        if (txt)
        {
            lv_msgbox_close(msgbox);
            lv_group_focus_freeze(g, false);
            lv_group_focus_obj(lv_obj_get_child(t1, 0));
            lv_obj_scroll_to(t1, 0, 0, LV_ANIM_OFF);
        }
    }
}

static void ta_event_cb(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_get_act();
    if (indev == NULL)
        return;
    lv_indev_type_t indev_type = lv_indev_get_type(indev);

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED && indev_type == LV_INDEV_TYPE_ENCODER)
    {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(kb);
        lv_group_set_editing((lv_group_t *)lv_obj_get_group(kb), kb);
        lv_obj_set_height(tv, LV_VER_RES / 2);
        lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    }

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
    {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_height(tv, LV_VER_RES);
    }
}
