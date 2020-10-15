#ifndef _SERVER_VM208_H
#define _SERVER_VM208_H
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ESPAsyncWebServer.h>
void handleDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final);
void startServer(void);
bool ON_AP_VM208_FILTER(AsyncWebServerRequest *request);
bool ON_STA_VM208_FILTER(AsyncWebServerRequest *request);
void saveConfig( void *pvParameters );
void sendSettings(AsyncWebServerRequest *request);
uint16_t convertToChannelId(uint8_t channel,int8_t interface,int8_t socket);
#endif