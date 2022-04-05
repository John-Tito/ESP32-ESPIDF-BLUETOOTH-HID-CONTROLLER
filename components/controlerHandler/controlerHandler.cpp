/* This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this software is
   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include "controlerHandler.hpp"

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
#include "nvs_flash.h"

#include "esp_hid_gap.h"
#include "esp_hidh.h"

static const char *TAG = "BTU";

uint8_t xInputRawData[17];
XboxControllerNotificationParser xInputParser;
QueueHandle_t xInputQueue;
EventGroupHandle_t xInputEventHandle = NULL;
SemaphoreHandle_t xInputSemaphore = NULL;

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
        if ((NULL != bda) && (NULL != xInputEventHandle) && xSemaphoreTake(xInputSemaphore, 0))
        {
            xInputRawData[0] = param->input.length;
            memcpy(&xInputRawData[1], param->input.data, param->input.length);
            if (0 == xInputParser.update(&xInputRawData[1], xInputRawData[0]))
            {
                // xQueueSend(xInputQueue, (void *)(&xInputRawData), 0);
                // xEventGroupSetBits(xInputEventHandle, XINPUT_UPDATE);
                // ESP_LOGI(TAG, "%d:%d", param->input.report_id, param->input.length);
            }
            else
            {
                ESP_LOGW(TAG, "invalid pack");
            }
            xSemaphoreGive(xInputSemaphore);
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
    while (NULL == xInputQueue)
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

bool controler_hid_init(void)
{
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ret = nvs_flash_erase();
        ESP_ERROR_CHECK(ret);

        ret = nvs_flash_init();
        ESP_ERROR_CHECK(ret);
    }
    ESP_ERROR_CHECK(ret);

#if CONFIG_BT_CLASSIC_ENABLED
    ret = esp_hid_gap_init(ESP_BT_MODE_BTDM);
#else
    ret = esp_hid_gap_init(ESP_BT_MODE_BLE);
#endif
    ESP_ERROR_CHECK(ret);

    ret = esp_ble_gattc_register_callback(esp_hidh_gattc_event_handler);
    ESP_ERROR_CHECK(ret);

    esp_hidh_config_t config = {
        .callback = hidh_callback,
    };
    ret = esp_hidh_init(&config);
    ESP_ERROR_CHECK(ret);

    xInputQueue = xQueueCreate(10, sizeof(xInputRawData));
    if (xInputQueue == NULL)
    {
        ESP_LOGE(TAG, "Error creating the queue");
        return false;
    }

    xInputSemaphore = xSemaphoreCreateMutex();
    if (xInputSemaphore == NULL)
    {
        ESP_LOGE(TAG, "Error creating Semaphore");
        return false;
    }

    xInputEventHandle = xEventGroupCreate();
    if (xInputEventHandle == NULL)
    {
        ESP_LOGE(TAG, "Error creating eventHandle");
        return false;
    }

    xTaskCreate(&hid_demo_task, "hid_task", 6 * 1024, NULL, 2, NULL);
    return true;
}
