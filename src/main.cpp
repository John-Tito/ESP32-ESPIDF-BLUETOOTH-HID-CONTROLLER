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

#define BDC0_PIN_A (32) // 左轮控制引脚 A
#define BDC0_PIN_B (33) // 左轮控制引脚 B

#define BDC1_PIN_A (25) // 右轮控制引脚 A
#define BDC1_PIN_B (26) // 右轮控制引脚 B

#define BDC_FREQ_HZ 1500 // 电机控制 PWM 频率

// 电机控制 PWM 单元
#define BDC_MCPWM_UNIT MCPWM_UNIT_0

#define BDC0_MCPWM_TIMER MCPWM_TIMER_0 // 左轮电机控制 PWM 单元定时器
#define BDC1_MCPWM_TIMER MCPWM_TIMER_1 // 右轮电机控制 PWM 单元定时器

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

    diff_duty_t duty_wheel;

    // 1.电机控制 PWM 引脚初始化
    ESP_LOGI(TAG_BDC, "init gpio");
    ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM0A, BDC0_PIN_A));
    ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM0B, BDC0_PIN_B));
    ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM1A, BDC1_PIN_A));
    ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM1B, BDC1_PIN_B));
    // 下面的引脚未使用，因此不进行初始化
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_CAP_0, BDC_PIN_CAP0));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_SYNC_0, BDC_PIN_SYNC0));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_FAULT_0, BDC_PIN_FAULT0));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_CAP_1, BDC_PIN_CAP1));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_SYNC_1, BDC_PIN_SYNC1));
    // ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM_FAULT_1, BDC_PIN_FAULT1));

    // 也可以使用下面的函数对引脚初始化
    // mcpwm_pin_config_t pin_config = {
    //     .mcpwm0a_out_num = BDC0_PIN_A,
    //     .mcpwm0b_out_num = BDC0_PIN_B,
    //     .mcpwm1a_out_num = BDC1_PIN_A,
    //     .mcpwm1b_out_num = BDC1_PIN_B,
    // };
    // mcpwm_set_pin(BDC_MCPWM_UNIT, &pin_config);

    // 2. 电机控制 PWM 以及定时器初始化
    ESP_LOGI(TAG_BDC, "init pwm");
    mcpwm_config_t pwm_config = {
        .frequency = BDC_FREQ_HZ,
        .cmpr_a = 0,
        .cmpr_b = 0,
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER};
    ESP_ERROR_CHECK(mcpwm_init(BDC_MCPWM_UNIT, BDC0_MCPWM_TIMER, &pwm_config));
    ESP_ERROR_CHECK(mcpwm_init(BDC_MCPWM_UNIT, BDC1_MCPWM_TIMER, &pwm_config));

    // 3. 载波配置，该例程不需要
    // ESP_LOGI(TAG_BDC, "init carrier");
    //  mcpwm_carrier_config_t chop_config;
    //  mcpwm_carrier_init(BDC_MCPWM_UNIT, MCPWM_TIMER_2, &chop_config);

    // 4. 死区配置，该例程不需要
    // mcpwm_deadtime_enable();

    // 5. 故障处理配置，该例程不需要
    //  mcpwm_fault_init();

    // 6. 同步配置，该例程不需要
    // mcpwm_sync_enable();

    // 7. 捕获配置，该例程不需要
    // mcpwm_capture_enable();
    // mcpwm_isr_register();

    // LED 控制定时器配置
    ledc_timer_config_t pwm3_timer_cfg = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_2,
        .freq_hz = 120,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&pwm3_timer_cfg);

    // LED 通道配置
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

    while (1)
    {
        // 获取信号量，
        if (pdTRUE == xSemaphoreTake(xInputSemaphore, 0))
        {
            // 判断是否有新的手柄数据
            if (xInputParser.outOfDate)
            {
                // 计算占空比
                get_duty(&duty_wheel);
                xInputParser.outOfDate = false;

                // 设置左轮占空比
                mcpwm_set_duty(BDC_MCPWM_UNIT, BDC0_MCPWM_TIMER, MCPWM_OPR_A, duty_wheel.duty_LA);
                mcpwm_set_duty(BDC_MCPWM_UNIT, BDC0_MCPWM_TIMER, MCPWM_OPR_B, duty_wheel.duty_LB);
                // mcpwm_set_signal_high(BDC_MCPWM_UNIT, BDC0_MCPWM_TIMER, MCPWM_OPR_B);

                // 设置右轮占空比
                mcpwm_set_duty(BDC_MCPWM_UNIT, BDC1_MCPWM_TIMER, MCPWM_OPR_A, duty_wheel.duty_RA);
                mcpwm_set_duty(BDC_MCPWM_UNIT, BDC1_MCPWM_TIMER, MCPWM_OPR_B, duty_wheel.duty_RB);
                // mcpwm_set_signal_high(BDC_MCPWM_UNIT, BDC1_MCPWM_TIMER, MCPWM_OPR_B);

                // 根据设置 LED 占空比
                ledc_set_duty(pwm3_channel_cfg.speed_mode, pwm3_channel_cfg.channel, (uint32_t)((uint32_t)duty_wheel.duty_LA * 1024 / 100));
                ledc_update_duty(pwm3_channel_cfg.speed_mode, pwm3_channel_cfg.channel);
            }
            xSemaphoreGive(xInputSemaphore);
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/**
 * @brief 计算差分信号的占空比
 *
 * @param diff_duty 差分信号的占空比
 */
static void get_duty(diff_duty_t *diff_duty)
{

    // 由左右扳机控制前后
    // 右扳机控制前进，计算前进的速度并归一化处理
    float up_speed = (((float)HOR_RES_MAX * (float)xInputParser.trigRT) / (float)xInputParser.TRIG_MAX);
    // 左扳机控制后退，计算后退的速度并归一化处理
    float down_speed = (((float)HOR_RES_MAX * (float)xInputParser.trigLT) / (float)xInputParser.TRIG_MAX);
    // 计算净前进速度
    float diff_speed = up_speed - down_speed;

    // 由左摇杆控制方向，计算左摇杆相对中间位置的偏移量
    float x = ((float)xInputParser.joyLHori - (float)xInputParser.JOY_MID);
    // 设置左右轮的初始速度
    float l_speed = 1.0, r_speed = 1.0;

    // 200 为死区区间
    if (x < -200)
    {
        // 左摇杆向左，表示向左转，则左侧车轮转动比右侧慢
        l_speed = 1.0 + x * 2 / (float)xInputParser.JOY_MAX;
    }
    else if (x > 200)
    {
        // 左摇杆向右，表示向右转，则右侧车轮转动比左侧慢
        r_speed = 1.0 - x * 2 / (float)xInputParser.JOY_MAX;
    }

    // 每侧的车轮由一对差分信号控制，通过控制两个信号的占空比来控制车轮的速度
    if (diff_speed >= 0)
    {
        diff_duty->duty_LA = l_speed * (diff_speed);
        diff_duty->duty_LB = 0;

        diff_duty->duty_RA = r_speed * (diff_speed);
        diff_duty->duty_RB = 0;
    }
    else
    {
        diff_duty->duty_LA = 0;
        diff_duty->duty_LB = l_speed * (-diff_speed);

        diff_duty->duty_RA = 0;
        diff_duty->duty_RB = r_speed * (-diff_speed);
    }
}
