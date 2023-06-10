
#ifndef _CONTROLERHANDLER_H_
#define _CONTROLERHANDLER_H_

#include "XboxControllerNotificationParser.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <iostream>
#include <string.h>

#define XINPUT_UPDATE ((TickType_t)0X000001)

extern bool controler_hid_init(void);

extern uint8_t xInputRawData[17];
extern QueueHandle_t xInputQueue;
extern EventGroupHandle_t xInputEventHandle;
extern SemaphoreHandle_t xInputSemaphore;
extern XboxControllerNotificationParser xInputParser;

#endif