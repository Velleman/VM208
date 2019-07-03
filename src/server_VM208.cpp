#include "server_VM208.h"

#define ARDUINOJSON_USE_LONG_LONG 1

#include "IO.hpp"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_eth.h"
#include "config_vm208.hpp"
#include <SPIFFS.h>
#include "global.hpp"
#include "config_vm208.hpp"
#include <Update.h>
const char *TAG = "SERVER";

// SKETCH BEGIN
AsyncWebServer server(80);

void sendBoardInfo(AsyncWebServerRequest *request);
void sendIOState(AsyncWebServerRequest *request);
String getMacAsString(uint8_t *mac);

void startServer()
{

  //#region server
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/boardinfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendBoardInfo(request);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendSettings(request);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendIOState(request);
  });

  server.on("/auth_settings", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->params() == 2)
    {
      config.setUserName(request->getParam(0)->value());
      config.setUserPw(request->getParam(1)->value());
      config.save();
    }
  });

  server.on("/relay", HTTP_POST, [](AsyncWebServerRequest *request) {
    String relay;
    String state;
    AsyncWebParameter *p = request->getParam(0);
    relay = p->value();
    p = request->getParam(1);
    state = p->value();

    Serial.println(relay);
    Serial.println(state);
    Channel *c = getChannelById(relay.toInt());

    if (state == "0")
    {
      c->turnOff();
    }
    else
    {
      c->turnOn();
    }

    sendIOState(request);
  });

  server.on("/mosfet", HTTP_POST, [](AsyncWebServerRequest *request) {
    String relay;
    String state;
    AsyncWebParameter *p = request->getParam(0);
    relay = p->value();
    p = request->getParam(1);
    state = p->value();
    Mosfet *m = getMosfetById(relay.toInt());
    if (state == "0")
    {
      m->turnOff();
    }
    else
    {
      m->turnOn();
    }

    sendIOState(request);
  });

  server.on("/pulse", HTTP_POST, [](AsyncWebServerRequest *request) {
    String relay;
    String time;
    AsyncWebParameter *p = request->getParam(0);
    relay = p->value();
    p = request->getParam(1);
    time = p->value();
    Channel *c = getChannelById(relay.toInt());
    c->activatePulse(time.toInt());
    sendIOState(request);
  });

  server.on("/timer", HTTP_POST, [](AsyncWebServerRequest *request) {
    String relay;
    String time;
    AsyncWebParameter *p = request->getParam(0);
    relay = p->value();
    p = request->getParam(1);
    time = p->value();
    Channel *c = getChannelById(relay.toInt());
    c->activateTimer(time.toInt());
    sendIOState(request);
  });

  server.on("/wifisave", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.printf("WifiSave\n");
    if (request->params() == 5)
    {
      config.setSSID(request->getParam(0)->value());
      config.setWifiPassword(request->getParam(1)->value());
      config.setBoardName(request->getParam(2)->value());
      config.setUserName(request->getParam(3)->value());
      config.setUserPw(request->getParam(4)->value());
      config.setFirstTime(false);
      config.save();
      Serial.printf("Wifi Saved\n");
      ESP.restart();
    }
  });

  server.on("/eth_ip_save", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.printf("eth_ip_save\n");
    if (request->params() == 6)
    {
      Serial.print("eth_ip_save ");
      Serial.println(request->getParam(0)->value());
      config.setETH_DHCPEnable(request->getParam(0)->value() == "true" ? true : false);
      config.setETH_IPAddress(request->getParam(1)->value());
      config.setETH_Gateway(request->getParam(2)->value());
      config.setETH_SubnetMask(request->getParam(3)->value());
      config.setETH_PrimaryDNS(request->getParam(4)->value());
      config.setETH_SecondaryDNS(request->getParam(5)->value());
      config.save();
      //applyEthNetworkSettings();
      request->send(200);
    }
    else
    {
      request->send(400);
    }
  });

  server.on("/wifi_ip_save", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.printf("wifi_ip_save\n");
    if (request->params() == 6)
    {
      Serial.print("wifi_ip_save ");
      Serial.println(request->getParam(0)->value());
      config.setWIFI_DHCPEnable(request->getParam(0)->value() == "true" ? true : false);
      config.setWIFI_IPAddress(request->getParam(1)->value());
      config.setWIFI_Gateway(request->getParam(2)->value());
      config.setWIFI_SubnetMask(request->getParam(3)->value());
      config.setWIFI_PrimaryDNS(request->getParam(4)->value());
      config.setWIFI_SecondaryDNS(request->getParam(5)->value());
      config.save();
      //applyWifiNetworkSettings();
      request->send(200);
    }
    else
    {
      request->send(400);
    }
  });

  server.on("/alarm", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->params() == 6)
    {
      Serial.printf("alarm save\n");
      uint8_t state = request->getParam(0)->value().toInt();
      uint8_t relais = request->getParam(1)->value().toInt();
      uint8_t day = request->getParam(2)->value().toInt();
      uint8_t hour = request->getParam(3)->value().toInt();
      uint8_t minute = request->getParam(4)->value().toInt();
      bool enabled = request->getParam(2)->value().toInt();
      Channel *c = getChannelById(relais);


      Alarm *a = c->getAlarm((day * 2)+state) ;

      Serial.print("Alarm state: ");
      Serial.println(state);

      Serial.print("Alarm day: ");
      Serial.println(day);

      Serial.print("Alarm hour: ");
      Serial.println(hour);

      Serial.print("Alarm minute: ");
      Serial.println(minute);
      
      Serial.print("Alarm Enabled ");
      Serial.println(enabled);

      a->setWeekday(day); //change to stupid english week system
      a->setHour(hour);
      a->setMinute(minute);
      a->setEnabled(enabled);
      config.save();
      request->send(200);
    }
    else
    {
      request->send(400);
    }
  });

  server.on("/doUpdate", HTTP_POST, [](AsyncWebServerRequest *request) {},
            [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
              handleDoUpdate(request, filename, index, data, len, final);
            });

  char user[32];
  char userpw[32];
  memset(user, 0, 32);
  memset(userpw, 0, 32);
  config.getUserName().toCharArray(user, 32);
  config.getUserPw().toCharArray(userpw, 32);
  Serial.println(user);
  Serial.println(userpw);
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html").setAuthentication(user, userpw).setFilter(ON_STA_VM208_FILTER);
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

void handleDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    log_i("Update");
    // if filename includes spiffs, update the spiffs partition
    int cmd = (filename.indexOf("spiffs") > 0) ? U_SPIFFS : U_FLASH;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
    {
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len)
  {
    Update.printError(Serial);
  }

  if (final)
  {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the VM208 reboots");
    response->addHeader("Refresh", "5");
    response->addHeader("Location", "/index.html");
    request->send(response);
    if (!Update.end(true))
    {
      Update.printError(Serial);
    }
    else
    {
      Serial.println("Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}

void sendBoardInfo(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  Serial.println(config.getBoardName());
  const size_t capacity = JSON_OBJECT_SIZE(5);
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject &root = jsonBuffer.createObject();
  root.set("time", (uint64_t)esp_timer_get_time());
  root.set("firmware", config.getVersion());
  root.set("name", config.getBoardName());
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_ETH);
  root.set("MAC_ETH", getMacAsString(mac));
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  root.set("MAC_WIFI", getMacAsString(mac));
  root.printTo(*response);
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

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  const size_t capacity = JSON_OBJECT_SIZE(16) + 190;
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject &root = jsonBuffer.createObject();
  Relay *relays = getRelays();
  Input *inputs;
  inputs = getCurrentInputs();
  Mosfet *m1 = getMosfetById(1);
  Mosfet *m2 = getMosfetById(2);
  Channel *c = getChannelById(1);
  root.set("relay1", relays[0].getState());
  root.set("relay2", relays[1].getState());
  root.set("relay3", relays[2].getState());
  root.set("relay4", relays[3].getState());
  root.set("relay5", relays[4].getState());
  root.set("relay6", relays[5].getState());
  root.set("relay7", relays[6].getState());
  root.set("relay8", relays[7].getState());
  root.set("relay9", relays[8].getState());
  root.set("relay10", relays[9].getState());
  root.set("relay11", relays[10].getState());
  root.set("relay12", relays[11].getState());
  root.set("isExtConnected", IsExtensionConnected());
  root.set("input", inputs[12].read());
  root.set("mosfet1", m1->getState());
  root.set("mosfet2", m2->getState());
  root.set("name1",c->getName());
  JsonArray& names = root.createNestedArray("names");
  for(int i =0;i<12;i++)
  {
    names.add((c+i)->getName());
  }
  
  root.printTo(*response);
  request->send(response);
}

void sendSettings(AsyncWebServerRequest *request)
{

  //AsyncResponseStream *response = request->beginResponseStream("application/json");
  AsyncJsonResponse *response = new AsyncJsonResponse();
  //DynamicJsonBuffer jsonBuffer;
  JsonObject &root = response->getRoot();

  root.set(config.USERNAME_KEY, config.getUserName());
  root.set(config.BOARDNAME_KEY, config.getBoardName());
  root.set(config.ETH_DHCPEN_KEY, config.getETH_DHCPEnable());
  root.set(config.ETH_IPADDR_KEY, config.getETH_IPAddress());          //TODO: change to interface settings
  root.set(config.ETH_GATEWAY_KEY, config.getETH_Gateway());           //TODO: change to interface settings
  root.set(config.ETH_SUBNETMASK_KEY, config.getETH_SubnetMask());     //TODO: change to interface settings
  root.set(config.ETH_PRIMARYDNS_KEY, config.getETH_PrimaryDNS());     //TODO: change to interface settings
  root.set(config.ETH_SECONDARYDNS_KEY, config.getETH_SecondaryDNS()); //TODO: change to interface settings
  root.set(config.WIFI_DHCPEN_KEY, config.getWIFI_DHCPEnable());
  root.set(config.WIFI_IPADDR_KEY, config.getWIFI_IPAddress());          //TODO: change to interface settings
  root.set(config.WIFI_GATEWAY_KEY, config.getWIFI_Gateway());           //TODO: change to interface settings
  root.set(config.WIFI_SUBNETMASK_KEY, config.getWIFI_SubnetMask());     //TODO: change to interface settings
  root.set(config.WIFI_PRIMARYDNS_KEY, config.getWIFI_PrimaryDNS());     //TODO: change to interface settings
  root.set(config.WIFI_SECONDARYDNS_KEY, config.getWIFI_SecondaryDNS()); //TODO: change to interface settings

  JsonArray &Channels = root.createNestedArray("Channels");

  for (int i = 0; i < 12; i++)
  {
    Channel *c = getChannelById(i + 1);
    JsonObject &Channel = Channels.createNestedObject();
    Channel[config.CHANNEL_NAME_KEY] = c->getName();

    JsonArray &Channels_alarms = Channel.createNestedArray("alarms");
    for (int j = 0; j < 14; j++)
    {
      Alarm *a = c->getAlarm(j);
      JsonObject &Channels_alarms_settings = Channels_alarms.createNestedObject();
      Channels_alarms_settings[config.ALARM_WEEKDAY_KEY] = a->getWeekday();
      Channels_alarms_settings[config.ALARM_HOUR_KEY] = a->getHour();
      Channels_alarms_settings[config.ALARM_MINUTE_KEY] = a->getMinute();
      Channels_alarms_settings[config.ALARM_STATE_KEY] = a->getState();
      Channels_alarms_settings[config.ALARM_ENABLED_KEY] = a->isEnabled();
    }
  }
  response->setLength();
  request->send(response);
}

bool ON_AP_VM208_FILTER(AsyncWebServerRequest *request)
{
  wifi_mode_t mode = WiFi.getMode();
  if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
  {
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
  if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
  {
    return false;
  }
  else
  {
    if (tcpip_adapter_is_netif_up(TCPIP_ADAPTER_IF_ETH) && gotETH_IP)
    {
      return true;
    }
    else
    {
      return true;
    }
  }
}
//#endregion server