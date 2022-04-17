/* This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this software is
   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include "argtable3/argtable3.h"
#include "driver/ledc.h"
#include "driver/mcpwm.h"
#include "esp_console.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "fastmath.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "hal/timer_hal.h"
#include "sdkconfig.h"
#include <iostream>
#include <string.h>

#include "controlerHandler.hpp"

/**********************
 *  MARCO DEFINES
 **********************/
#define HOR_RES_MAX 100
#define VER_RES_MAX 100

#define BDC0_PIN_A (32)
#define BDC0_PIN_B (33)

#define BDC1_PIN_A (25)
#define BDC1_PIN_B (26)

#define BDC0_FREQ_HZ 1500
#define BDC1_FREQ_HZ 1500

#define BDC_MCPWM_UNIT MCPWM_UNIT_0

#define BDC0_MCPWM_TIMER MCPWM_TIMER_0
#define BDC1_MCPWM_TIMER MCPWM_TIMER_1

#define BDC_PIN_CAP0 0
#define BDC_PIN_SYNC0 0
#define BDC_PIN_FAULT0 0

#define BDC_PIN_CAP1 0
#define BDC_PIN_SYNC1 0
#define BDC_PIN_FAULT1 0

/**********************
 *  TYPE DEFINES
 **********************/
typedef struct
{
    float duty_LA, duty_LB, duty_RA, duty_RB;
} diff_duty_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void task_pwm(void *param);
static void get_duty(diff_duty_t *diff_duty);

/**********************
 *  STATIC VARIABLES
 **********************/
static const char *TAG_INIT = "init";
static const char *TAG_BDC = "BDC";

extern "C" void app_main(void)
{
    if (!controler_hid_init())
        ESP_LOGI(TAG_INIT, "err when init hid");
    xTaskCreate(&task_pwm, "ui_loop", 10 * 1024, NULL, 2, NULL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void task_pwm(void *param)
{
    // 1. mcpwm gpio initialization
    ESP_LOGI(TAG_BDC, "init gpio");
    ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM0A, BDC0_PIN_A));
    ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM0B, BDC0_PIN_B));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_CAP_0, BDC_PIN_CAP0));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_SYNC_0, BDC_PIN_SYNC0));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_FAULT_0, BDC_PIN_FAULT0));

    ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM1A, BDC1_PIN_A));
    ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM1B, BDC1_PIN_B));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_CAP_1, BDC_PIN_CAP1));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_SYNC_1, BDC_PIN_SYNC1));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_FAULT_1, BDC_PIN_FAULT1));

    // mcpwm_pin_config_t pin_config;
    // mcpwm_set_pin(BDC_MCPWM_UNIT, &pin_config);

    // 2. initialize mcpwm configuration
    ESP_LOGI(TAG_BDC, "init pwm");
    mcpwm_config_t pwm_config = {
        .frequency = BDC0_FREQ_HZ,
        .cmpr_a = 0,
        .cmpr_b = 0,
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER};
    ESP_ERROR_CHECK(mcpwm_init(BDC_MCPWM_UNIT, BDC0_MCPWM_TIMER, &pwm_config));
    ESP_ERROR_CHECK(mcpwm_init(BDC_MCPWM_UNIT, BDC1_MCPWM_TIMER, &pwm_config));

    // 3. carrier configuration
    // ESP_LOGI(TAG_BDC, "init carrier");
    //  mcpwm_carrier_config_t chop_config;
    //  mcpwm_carrier_init(BDC_MCPWM_UNIT, MCPWM_TIMER_2, &chop_config);

    // 4. deadtime configuration
    // mcpwm_deadtime_enable();

    // 5. enable fault condition
    //  mcpwm_fault_init();

    // 6. Syncronization configuration
    // mcpwm_sync_enable();

    // 7. Capture configuration
    // mcpwm_capture_enable();
    // mcpwm_isr_register();

    ledc_timer_config_t pwm3_timer_cfg = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_2,
        .freq_hz = 120,
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

    diff_duty_t duty_wheel;
    while (1)
    {
        if (pdTRUE == xSemaphoreTake(xInputSemaphore, 0))
        {
            if (xInputParser.outOfDate)
            {
                get_duty(&duty_wheel);
                xInputParser.outOfDate = false;

                mcpwm_set_duty(BDC_MCPWM_UNIT, BDC0_MCPWM_TIMER, MCPWM_OPR_A, duty_wheel.duty_LA);
                mcpwm_set_duty(BDC_MCPWM_UNIT, BDC0_MCPWM_TIMER, MCPWM_OPR_B, duty_wheel.duty_LB);
                // mcpwm_set_signal_high(BDC_MCPWM_UNIT, BDC0_MCPWM_TIMER, MCPWM_OPR_B);

                mcpwm_set_duty(BDC_MCPWM_UNIT, BDC1_MCPWM_TIMER, MCPWM_OPR_A, duty_wheel.duty_RA);
                mcpwm_set_duty(BDC_MCPWM_UNIT, BDC1_MCPWM_TIMER, MCPWM_OPR_B, duty_wheel.duty_RB);
                // mcpwm_set_signal_high(BDC_MCPWM_UNIT, BDC1_MCPWM_TIMER, MCPWM_OPR_B);

                ledc_set_duty(pwm3_channel_cfg.speed_mode, pwm3_channel_cfg.channel, (uint32_t)((uint32_t)duty_wheel.duty_LA * 1024 / 100));
                ledc_update_duty(pwm3_channel_cfg.speed_mode, pwm3_channel_cfg.channel);
            }
            xSemaphoreGive(xInputSemaphore);
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

static void get_duty(diff_duty_t *diff_duty)
{

    float up_speed, down_speed;
    up_speed = (((float)HOR_RES_MAX * (float)xInputParser.trigRT) / (float)xInputParser.TRIG_MAX);

    down_speed = (((float)HOR_RES_MAX * (float)xInputParser.trigLT) / (float)xInputParser.TRIG_MAX);
    diff_duty->duty_RB = 0;

    float x = ((float)xInputParser.joyLHori - (float)xInputParser.JOY_MID);

    float l_speed = 0.0, r_speed = 0.0;
    if (x < -200)
    {
        l_speed = 1.0 + x * 2 / (float)xInputParser.JOY_MAX;
        r_speed = 1.0;
    }
    else if (x > 200)
    {
        l_speed = 1.0;
        r_speed = 1.0 - x * 2 / (float)xInputParser.JOY_MAX;
    }
    else
    {
        l_speed = 1.0;
        r_speed = 1.0;
    }

    if ((up_speed - down_speed) >= 0)
    {
        diff_duty->duty_LA = l_speed * (up_speed - down_speed);
        diff_duty->duty_LB = 0;

        diff_duty->duty_RA = r_speed * (up_speed - down_speed);
        diff_duty->duty_RB = 0;
    }
    else
    {
        diff_duty->duty_LA = 0;
        diff_duty->duty_LB = l_speed * (down_speed - up_speed);

        diff_duty->duty_RA = 0;
        diff_duty->duty_RB = r_speed * (down_speed - up_speed);
    }
}