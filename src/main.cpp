/* This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this software is
   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include "driver/ledc.h"
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

#define HOR_RES_MAX 1024
#define VER_RES_MAX 1024

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void task_pwm(void *param);
static void get_duty(uint16_t *x, uint16_t *y);

/**********************
 *  STATIC VARIABLES
 **********************/
static const char *TAG_INIT = "init";

extern "C" void app_main(void)
{
    if (!controler_hid_init())
        ESP_LOGI(TAG_INIT, "err when init hid");
    xTaskCreate(&task_pwm, "ui_loop", 10 * 1024, NULL, 2, NULL);
}

/* create frame buffer */
static void task_pwm(void *param)
{

    ledc_timer_config_t pwm1_timer_cfg = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&pwm1_timer_cfg);

    ledc_channel_config_t pwm1_channel_cfg = {
        .gpio_num = 25,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&pwm1_channel_cfg);

    ledc_timer_config_t pwm2_timer_cfg = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&pwm2_timer_cfg);

    ledc_channel_config_t pwm2_channel_cfg = {
        .gpio_num = 26,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&pwm2_channel_cfg);

    ledc_timer_config_t pwm3_timer_cfg = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_2,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&pwm3_timer_cfg);

    ledc_channel_config_t pwm3_channel_cfg = {
        .gpio_num = 5,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_2,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_2,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&pwm3_channel_cfg);

    // ledc_fade_func_install(0);

    uint16_t duty_x, duty_y;
    while (1)
    {
        if (pdTRUE == xSemaphoreTake(xInputSemaphore, 0))
        {
            if (xInputParser.outOfDate)
            {
                get_duty(&duty_x, &duty_y);
                xInputParser.outOfDate = false;
                // ESP_LOGI("pwm", "dx:%u,dy:%u", duty_x, duty_y);
                ledc_set_duty(pwm1_channel_cfg.speed_mode, pwm1_channel_cfg.channel, duty_x);
                ledc_update_duty(pwm1_channel_cfg.speed_mode, pwm1_channel_cfg.channel);

                ledc_set_duty(pwm2_channel_cfg.speed_mode, pwm2_channel_cfg.channel, duty_y);
                ledc_update_duty(pwm2_channel_cfg.speed_mode, pwm2_channel_cfg.channel);

                ledc_set_duty(pwm3_channel_cfg.speed_mode, pwm3_channel_cfg.channel, duty_x);
                ledc_update_duty(pwm3_channel_cfg.speed_mode, pwm3_channel_cfg.channel);
            }
            xSemaphoreGive(xInputSemaphore);
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void get_duty(uint16_t *x, uint16_t *y)
{
    *x = (((uint32_t)HOR_RES_MAX * (uint32_t)xInputParser.trigLT) / xInputParser.TRIG_MAX);
    *y = (((uint32_t)HOR_RES_MAX * (uint32_t)xInputParser.trigRT) / xInputParser.TRIG_MAX);
}