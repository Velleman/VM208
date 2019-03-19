#ifndef _WIFICONFIG_H_
#define _WIFICONFIG_H_

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"

EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const int GOTIP_BIT = BIT2;


void smartconfig_example_task(void * parm);
//esp_err_t event_handler(void *ctx, system_event_t *event);
void initialise_wifi(void);
void sc_callback(smartconfig_status_t status, void *pdata);
void wificonfig_task(void * parm);

#endif