#include "server_VM208.h"

#define ARDUINOJSON_USE_LONG_LONG 1

#include "IO.h"

#include "ArduinoJson.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_eth.h"
#include "configuration.h"
#include <SPIFFS.h>
const char *TAG = "SERVER";

// SKETCH BEGIN
AsyncWebServer server(80);


void sendBoardInfo(AsyncWebServerRequest *request);
void sendIOState(AsyncWebServerRequest *request);
String getMacAsString(uint8_t *mac);
const char *http_username = "admin";
const char *http_password = "admin";

extern int gotETH_IP;
extern Configuration config;

void startServer()
{
  
  //#region server
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/boardinfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendBoardInfo(request);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendIOState(request);
  });

  server.on("/relay", HTTP_POST, [](AsyncWebServerRequest *request){
    String relay;
    String state;
    AsyncWebParameter *p = request->getParam(0);
    relay = p->value();
    p = request->getParam(1);
    state = p->value();

    Serial.println(relay);
    Serial.println(state);
    setRelay(relay.toInt()-1, state.toInt()? RELAY_ON: RELAY_OFF);
    sendIOState(request);
  });

  server.on("/wifisave", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.printf("WifiSave\n");
    String ssid;
    String pw;
    String name;

    AsyncWebParameter *p = request->getParam(0);
    ssid = p->value();
    p = request->getParam(1);
    pw = p->value();
    p = request->getParam(2);
    name = p->value();
    config.setSSID(ssid);
    config.setWifiPassword(pw);
    config.setBoardName(name);
    ESP_LOGI(TAG,"TEST1");
    xTaskCreate(saveConfig,"saveconfig",4096,NULL,tskIDLE_PRIORITY+2,NULL);
  });

  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html").setAuthentication(http_username, http_password).setFilter(ON_STA_VM208_FILTER);
  server.serveStatic("/", SPIFFS, "/ap/").setDefaultFile("index.html").setFilter(ON_AP_VM208_FILTER);

  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.printf("NOT_FOUND: ");
    if (request->method() == HTTP_GET)
      Serial.printf("GET");
    else if (request->method() == HTTP_POST)
      Serial.printf("POST");
    else if (request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if (request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if (request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if (request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if (request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if (request->contentLength())
    {
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++)
    {
      AsyncWebHeader *h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for (i = 0; i < params; i++)
    {
      AsyncWebParameter *p = request->getParam(i);
      if (p->isFile())
      {
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      }
      else if (p->isPost())
      {
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
      else
      {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char *)data);
    if (final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char *)data);
    if (index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });
  server.begin();
}

void sendBoardInfo(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  ESP_LOGI(TAG,"create buffer boardinfo");
  Serial.println(config.getBoardName());
  const size_t capacity = JSON_OBJECT_SIZE(5);
  DynamicJsonBuffer jsonBuffer(capacity);
  ESP_LOGI(TAG,"created buffer boardinfo");
  JsonObject &root = jsonBuffer.createObject();
  ESP_LOGI(TAG,"debug");
  root.set("time", (uint64_t)esp_timer_get_time());
  ESP_LOGI(TAG,"debug");
  root.set("firmware", "0.1.0");
  ESP_LOGI(TAG,"debug");
  root.set("name", config.getBoardName());
  ESP_LOGI(TAG,"debug");
  uint8_t mac[6];
  ESP_LOGI(TAG,"debug");
  esp_read_mac(mac, ESP_MAC_ETH);
  ESP_LOGI(TAG,"debug");
  root.set("MAC_ETH", getMacAsString(mac));
  ESP_LOGI(TAG,"debug");
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  ESP_LOGI(TAG,"debug");
  root.set("MAC_WIFI", getMacAsString(mac));
  ESP_LOGI(TAG,"debug");
  ESP_LOGI(TAG,"print buffer boardinfo");
  ESP_LOGI(TAG,"debug");
  root.printTo(*response);
  ESP_LOGI(TAG,"printed buffer boardinfo");
  request->send(response);
}

String getMacAsString(uint8_t *mac)
{
  String str;
  String str1(mac[0], HEX);
  String str2(mac[1], HEX);
  String str3(mac[2], HEX);
  String str4(mac[3], HEX);
  String str5(mac[4], HEX);
  String str6(mac[5], HEX);
  str = str1 + ":" + str2 + ":" + str3 + ":" + str4 + ":" + str5 + ":" + str6;
  return str;
}

void sendIOState(AsyncWebServerRequest *request)
{
  //ESP_LOGI(TAG,"enter state");
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  //ESP_LOGI(TAG,"create buffer send state");
  const size_t capacity = JSON_OBJECT_SIZE(13) + 190;
  DynamicJsonBuffer jsonBuffer(capacity);
  //ESP_LOGI(TAG,"buffer created send state");
  JsonObject &root = jsonBuffer.createObject();
  relay_t relays[12];
  getRelays(relays, 12);

  root.set("relay1", relays[0].state);
  root.set("relay2", relays[1].state);
  root.set("relay3", relays[2].state);
  root.set("relay4", relays[3].state);
  root.set("relay5", relays[4].state);
  root.set("relay6", relays[5].state);
  root.set("relay7", relays[6].state);
  root.set("relay8", relays[7].state);
  root.set("relay9", relays[8].state);
  root.set("relay10", relays[9].state);
  root.set("relay11", relays[10].state);
  root.set("relay12", relays[11].state);
  root.set("isExtConnected", IsExtensionConnected());
  //ESP_LOGI(TAG,"printto json send state");
  root.printTo(*response);
  //ESP_LOGI(TAG,"printto json done send state");
  request->send(response);
}

bool ON_AP_VM208_FILTER(AsyncWebServerRequest *request)
{
  wifi_mode_t mode = WiFi.getMode();
  if(mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA){
    return true;
  }
  else
  {
    return false;
  }
  
}

bool ON_STA_VM208_FILTER(AsyncWebServerRequest *request)
{
  
  wifi_mode_t mode = WiFi.getMode();
  if(mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA){
    return false;
  }
  else
  {
    if(tcpip_adapter_is_netif_up(TCPIP_ADAPTER_IF_ETH) && gotETH_IP)
    {
      return true;
    }
    else
    {
      return true;
    }
  }
  
  
}

void saveConfig( void *pvParameters )
{
  ESP_LOGI(TAG,"TEST1");
  config.save();
  vTaskDelete( NULL );
}
//#endregion server