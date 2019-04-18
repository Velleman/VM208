#ifndef _SERVER_VM208_H
#define _SERVER_VM208_H

#include <ESPAsyncWebServer.h>

void startServer(void);
bool ON_AP_VM208_FILTER(AsyncWebServerRequest *request);
bool ON_STA_VM208_FILTER(AsyncWebServerRequest *request);
void saveConfig( void *pvParameters );
#endif