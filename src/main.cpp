/* This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this software is
   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include <iostream>
#include <string.h>

#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_event.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "esp_hid_gap.h"
#include "esp_hidh.h"

#include "USER_LOGI.h"

#include "./XboxControllerNotificationParser/src/XboxControllerNotificationParser.hpp" // This will pull in our MPG implementation

static const char *TAG = "TEST";
XboxControllerNotificationParser parser;

QueueHandle_t queue;
input_report_raw_t raw_data;
void hid_demo_task(void *pvParameters);
void task_printout(void *param);
void hidh_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    esp_hidh_event_t event = (esp_hidh_event_t)id;
    esp_hidh_event_data_t *param = (esp_hidh_event_data_t *)event_data;

    switch (event)
    {
    case ESP_HIDH_OPEN_EVENT:
    {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->open.dev);
        ESP_LOGI(TAG, ESP_BD_ADDR_STR " OPEN: %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->open.dev));
        esp_hidh_dev_dump(param->open.dev, stdout);
        break;
    }
    case ESP_HIDH_BATTERY_EVENT:
    {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->battery.dev);
        ESP_LOGI(TAG, ESP_BD_ADDR_STR " BATTERY: %d%%", ESP_BD_ADDR_HEX(bda), param->battery.level);
        break;
    }
    case ESP_HIDH_INPUT_EVENT:
    {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->input.dev);
        if (NULL != bda)
        {
            memcpy(raw_data.data, param->input.data, param->input.length);
            raw_data.length = param->input.length;
            xQueueSend(queue, (void *)(&raw_data), 0);
        }
        break;
    }
    case ESP_HIDH_FEATURE_EVENT:
    {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->feature.dev);
        ESP_LOGI(TAG, ESP_BD_ADDR_STR " FEATURE: %8s, MAP: %2u, ID: %3u, Len: %d", ESP_BD_ADDR_HEX(bda), esp_hid_usage_str(param->feature.usage), param->feature.map_index, param->feature.report_id, param->feature.length);
        ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
        break;
    }
    case ESP_HIDH_CLOSE_EVENT:
    {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->close.dev);
        ESP_LOGI(TAG, ESP_BD_ADDR_STR " CLOSE: '%s' %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->close.dev), esp_hid_disconnect_reason_str(esp_hidh_dev_transport_get(param->close.dev), param->close.reason));
        // MUST call this function to free all allocated memory by this device
        esp_hidh_dev_free(param->close.dev);
        break;
    }
    default:
        ESP_LOGI(TAG, "EVENT: %d", event);
        break;
    }
}

#define SCAN_DURATION_SECONDS 5

void hid_demo_task(void *pvParameters)
{
    size_t results_len = 0;
    esp_hid_scan_result_t *results = NULL;
    while (NULL == queue)
    {
        vTaskDelay(1000);
        ESP_LOGI(TAG, "wait queue...");
    }

    ESP_LOGI(TAG, "SCAN...");
    // start scan for HID devices
    esp_hid_scan(SCAN_DURATION_SECONDS, &results_len, &results);
    ESP_LOGI(TAG, "SCAN: %u results", results_len);
    if (results_len)
    {
        esp_hid_scan_result_t *r = results;
        esp_hid_scan_result_t *cr = NULL;
        while (r)
        {
            ESP_LOGI(TAG, "%s: " ESP_BD_ADDR_STR "\n", (r->transport == ESP_HID_TRANSPORT_BLE) ? "BLE" : "BT ", ESP_BD_ADDR_HEX(r->bda));
            ESP_LOGI(TAG, "RSSI: %d\n", r->rssi);
            ESP_LOGI(TAG, "USAGE: %s\n", esp_hid_usage_str(r->usage));
            if (r->transport == ESP_HID_TRANSPORT_BLE)
            {
                cr = r;
                ESP_LOGI(TAG, "APPEARANCE: 0x%04x\n", r->ble.appearance);
                ESP_LOGI(TAG, "ADDR_TYPE: %s\n", ble_addr_type_str(r->ble.addr_type));
            }
            else
            {
                cr = r;

                ESP_LOGI(TAG, "COD: %s[", esp_hid_cod_major_str(r->bt.cod.major));
                esp_hid_cod_minor_print(r->bt.cod.minor, stdout);
                printf("]\n");

                ESP_LOGI(TAG, "srv 0x%03x\n", r->bt.cod.service);

                ESP_LOGI(TAG, "uuid:");
                print_uuid(&r->bt.uuid);
                printf("\n");
            }

            ESP_LOGI(TAG, "NAME:%s", r->name ? r->name : "");
            printf("\n");
            r = r->next;
        }
        if (cr)
        {
            // open the last result
            esp_hidh_dev_open(cr->bda, cr->transport, cr->ble.addr_type);
        }
        // free the results
        esp_hid_scan_results_free(results);
    }
    vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
#if CONFIG_BT_CLASSIC_ENABLED
    ESP_ERROR_CHECK(esp_hid_gap_init(ESP_BT_MODE_BTDM));
#else
    ESP_ERROR_CHECK(esp_hid_gap_init(ESP_BT_MODE_BLE));
#endif
    ESP_ERROR_CHECK(esp_ble_gattc_register_callback(esp_hidh_gattc_event_handler));
    esp_hidh_config_t config = {
        .callback = hidh_callback,
    };
    ESP_ERROR_CHECK(esp_hidh_init(&config));
    queue = xQueueCreate(10, sizeof(input_report_raw_t));
    if (queue == NULL)
    {
        ESP_LOGE(TAG, "Error creating the queue");
    }
    xTaskCreate(&hid_demo_task, "hid_task", 6 * 1024, NULL, 2, NULL);
    xTaskCreate(&task_printout, "key_out", 6 * 1024, NULL, 2, NULL);
}

input_report_raw_t raw1;
void task_printout(void *param)
{
    while (NULL == queue)
    {
        vTaskDelay(1000);
        ESP_LOGI(TAG, "wait queue...");
    }
    while (1)
    {
        xQueueReceive(queue, &raw1, portMAX_DELAY);
        if (parser.update(raw1.data, raw1.length))
            ESP_LOGI(TAG, "LT:%d", parser.trigLT);
        ESP_LOG_BUFFER_HEX(TAG, raw1.data, raw1.length);
        vTaskDelay(50);
    }
}
