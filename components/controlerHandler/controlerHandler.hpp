
#ifndef _CONTROLERHANDLER_H_
#define _CONTROLERHANDLER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <iostream>
#include <string.h>

extern QueueHandle_t controllerQueue;
extern bool controler_hid_init(void);

#endif