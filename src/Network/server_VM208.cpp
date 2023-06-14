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
#include <Update.h>
#include <ETH.h>
#include "mail.hpp"
#include "network_VM208.hpp"
#include <DNSServer.h>
#include "PulseAndTimer.hpp"
#include "time_VM208.hpp"
const char *TAG = "SERVER";

// SKETCH BEGIN
AsyncWebServer server(80);

void sendBoardInfo(AsyncWebServerRequest *request);
void sendIOState(AsyncWebServerRequest *request);
void sendIOState(AsyncWebServerRequest *request, int8_t interface, uint8_t socket);
// String getMacAsString(uint8_t *mac);

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request)
  {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>");
    response->print("<p>This is out captive portal front page.</p>");
    response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
    response->printf("<p>Try opening <a href='http://%s'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());
    response->print("</body></html>");
    request->send(response);
  }
};

void startServer()
{

  // #region server
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(ESP.getFreeHeap())); });

  server.on("/boardinfo", HTTP_GET, [](AsyncWebServerRequest *request)
            { sendBoardInfo(request); });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
            { sendSettings(request); });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->params() == 2)
              {
                sendIOState(request, request->getParam(0)->value().toInt(), request->getParam(1)->value().toInt());
              } });

  server.on("/auth_settings", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->params() == 2)
              {
                config.setUserName(request->getParam(0)->value());
                config.setUserPw(request->getParam(1)->value());
                config.save();
                ESP.restart();
              } });

  server.on("/relay", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              long relay;
              String state;
              AsyncWebParameter *p = request->getParam(0);
              relay = p->value().toInt();
              p = request->getParam(1);
              state = p->value();
              auto interface = request->getParam(2)->value().toInt();
              auto socket = request->getParam(3)->value().toInt();
              //relay = convertToChannelId(relay, interface, socket);
              Serial.printf("Channel is :%d", relay);
              if (xSemaphoreTake(g_Mutex, 100 / portTICK_PERIOD_MS))
              {
                if (interface != 0 && interface != -1)
                {
                  VM208EX *module = (VM208EX *)mm.getModuleFromInterface(interface - 1, socket - 1);

                  if (state.toInt())
                    module->turnOnChannel(relay - 1);
                  else
                    module->turnOffChannel(relay - 1);
                }
                else
                {
                  if (interface == -1)
                  {
                    VM208 *module = mm.getBaseModule();
                    if (state.toInt())
                      module->turnOnChannel(relay - 1);
                    else
                      module->turnOffChannel(relay - 1);
                  }
                  else
                  {
                    VM208EX *module = (VM208EX *)mm.getModule(1);
                    if (state.toInt())
                      module->turnOnChannel(relay - 1);
                    else
                      module->turnOffChannel(relay - 1);
                  }
                }
                //state.toInt() ? channel->turnOn() : channel->turnOff();
                xSemaphoreGive(g_Mutex);
              }
              xQueueSend(pulseStopQueue, &relay, 0);
              xQueueSend(timerStopQueue, &relay, 0);
              sendIOState(request, interface, socket); });

  server.on("/mosfet", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              uint8_t mosfet;
              uint8_t state;
              mosfet = request->getParam(0)->value().toInt();
              state = request->getParam(1)->value().toInt();
              if(xSemaphoreTake(g_Mutex,100/portTICK_PERIOD_MS))
              {
              if (mosfet == 1)
              {
                if (state == 1)
                  mm.getBaseModule()->turnOnMosfet1();
                else
                {
                  mm.getBaseModule()->turnOffMosfet1();
                }
              }
              else
              {
                if (state == 1)
                  mm.getBaseModule()->turnOnMosfet2();
                else
                {
                  mm.getBaseModule()->turnOffMosfet2();
                }
              }
              }
              xSemaphoreGive(g_Mutex);
              sendIOState(request); });

  server.on("/pulse", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              long relay;
              String time;
              AsyncWebParameter *p = request->getParam(0);
              relay = p->value().toInt();
              p = request->getParam(1);
              time = p->value();
              auto interface = request->getParam(2)->value().toInt();
              auto socket = request->getParam(3)->value().toInt();
              relay = convertToChannelId(relay, interface, socket);
              TimeParameters_t params;
              params.id = relay;
              params.time = time.toInt();
              //send params to pulse queue
              xQueueSend(pulseQueue, &params, 0);
              request->send(200); });

  server.on("/stoppulse", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              long relay;
              AsyncWebParameter *p = request->getParam(0);
              relay = p->value().toInt();
              auto interface = request->getParam(1)->value().toInt();
              auto socket = request->getParam(1)->value().toInt();
              relay = convertToChannelId(relay, interface, socket);
              //send params to pulse queue
              xSemaphoreTake(g_Mutex, 100 / portTICK_PERIOD_MS);
              mm.getChannel(relay)->toggle();
              xSemaphoreGive(g_Mutex);
              xQueueSend(pulseStopQueue, &relay, 0);
              request->send(200); });

  server.on("/stoptimer", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              long relay;
              AsyncWebParameter *p = request->getParam(0);
              relay = p->value().toInt();
              auto interface = request->getParam(1)->value().toInt();
              auto socket = request->getParam(1)->value().toInt();
              relay = convertToChannelId(relay, interface, socket);
              xSemaphoreTake(g_Mutex, 100 / portTICK_PERIOD_MS);
              mm.getChannel(relay)->toggle();
              xSemaphoreGive(g_Mutex);
              //send params to pulse queue
              xQueueSend(timerStopQueue, &relay, 0);

              request->send(200); });

  server.on("/timer", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              String time;
              AsyncWebParameter *p = request->getParam(0);
              auto relay = p->value().toInt();
              p = request->getParam(1);
              time = p->value();
              auto interface = request->getParam(2)->value().toInt();
              auto socket = request->getParam(3)->value().toInt();
              relay = convertToChannelId(relay, interface, socket);
              TimeParameters_t params;
              params.id = relay;
              params.time = time.toInt();
              // send params to timer queue
              xQueueSend(timerQueue, &params, 0);

              request->send(200); });

  server.on("/getalarms", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              /*JsonArray &Channels = root.createNestedArray("Channels");
  VM208TimeChannel *c;
  Alarm *a;

  for (int i = 0; i < 12; i++)
  {
    c = (VM208TimeChannel *)getRelayChannelById(i + 1);
    JsonObject &Channel = Channels.createNestedObject();
    Channel[config.CHANNEL_NAME_KEY] = c->getName();
    JsonArray &Channels_alarms = Channel.createNestedArray("alarms");
    for (int j = 0; j < 14; j++)
    {
      a = c->getAlarm(j);
      JsonObject &Channels_alarms_settings = Channels_alarms.createNestedObject();
      Channels_alarms_settings[config.ALARM_WEEKDAY_KEY] = a->getWeekday();
      Channels_alarms_settings[config.ALARM_HOUR_KEY] = a->getHour();
      Channels_alarms_settings[config.ALARM_MINUTE_KEY] = a->getMinute();
      Channels_alarms_settings[config.ALARM_STATE_KEY] = a->getState();
      Channels_alarms_settings[config.ALARM_ENABLED_KEY] = a->isEnabled();
    }
  }*/
              uint16_t id = request->getParam(0)->value().toInt();
              // Send the alarms
              AsyncResponseStream *response = request->beginResponseStream("application/json");
              DynamicJsonBuffer jsonBuffer;
              JsonObject &root = jsonBuffer.createObject();
              JsonObject &Channel = root.createNestedObject("Channel");
              JsonArray &channel_alarms = Channel.createNestedArray("alarms");
              ChannelShedule *shedule = config.getShedule(id - 1);
              for (int j = 0; j < 14; j++)
              {
                JsonObject &Channels_alarms_settings = channel_alarms.createNestedObject();
                auto dow = j > 6 ? j - 7 : j;
                bool onOff = (j < 7);
                Serial.print("Day of week is: ");
                Serial.println(dow);
                Serial.print("onOff is ");
                Serial.println(onOff);
                Channels_alarms_settings["dow"] = dow; // shedule->getShedule(dow,onOff)->dateTime.tm_wday;
                Channels_alarms_settings["hour"] = shedule->getShedule(dow, onOff)->hour;
                Channels_alarms_settings["minute"] = shedule->getShedule(dow, onOff)->minute;
                Channels_alarms_settings["onoff"] = onOff;
                Channels_alarms_settings["enabled"] = shedule->getShedule(dow, onOff)->enable;
              }
              root.printTo(*response);
              request->send(response);
              jsonBuffer.clear();
              // request->send(200);
            });

  server.on("/setalarm", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->params() == 6)
              {
                Serial.printf("alarm save\n");
                uint8_t state = request->getParam(0)->value().toInt();
                uint8_t relais = request->getParam(1)->value().toInt();
                uint8_t day = request->getParam(2)->value().toInt();
                uint8_t hour = request->getParam(3)->value().toInt();
                uint8_t minute = request->getParam(4)->value().toInt();
                bool enabled = request->getParam(5)->value().toInt();
                tm dateTime;
                dateTime.tm_wday = day;
                dateTime.tm_hour = hour;
                dateTime.tm_min = minute;
                config.setShedule(relais, day, hour, minute, state, enabled);
                request->send(200);
              }
              else
              {
                request->send(400);
              } });

  server.on("/email_settings", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->params() == 6)
              {
                config.setEmailServer(request->getParam(0)->value());
                config.setEmailPort(request->getParam(1)->value());
                config.setEmailUser(request->getParam(2)->value());
                config.setEmailPW(request->getParam(3)->value());
                config.setEmailRecipient(request->getParam(4)->value());
                config.setEmailSubject(request->getParam(5)->value());
                config.saveEmailSettings();
                request->send(200, "text/plain", "OK");
              }
              else
              {
                request->send(400);
              } });

  server.on("/testmail", HTTP_POST, [](AsyncWebServerRequest *request)
            { sendTestMail(); });

  server.on("/wifisave", HTTP_POST, [](AsyncWebServerRequest *request)
            {
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
                Serial.println(config.getSSID());
                Serial.println(config.getWifiPassword());
                Serial.println("Wifi Saved\n");
                ESP.restart();
              }
              else
              {
                request->send(400);
              } });

  server.on("/wifi_creds_save", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              Serial.printf("WifiSave\n");
              if (request->params() == 2)
              {
                config.setSSID(request->getParam(0)->value());
                config.setWifiPassword(request->getParam(1)->value());
                config.save();
                startWifi();
                Serial.printf("Wifi Saved\n");
              }
              else
              {
                request->send(400);
              } });

  server.on("/eth_ip_save", HTTP_POST, [](AsyncWebServerRequest *request)
            {
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
                applyEthNetworkSettings();
                request->send(200);
              }
              else
              {
                request->send(400);
              } });

  server.on("/wifi_ip_save", HTTP_POST, [](AsyncWebServerRequest *request)
            {
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
                applyWifiNetworkSettings();
                request->send(200);
              }
              else
              {
                request->send(400);
              } });

  server.on("/notif_setting", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              Serial.printf("notif_setting\n");
              if (request->params() == 4)
              {
                config.setNotificationBoot(request->getParam(0)->value() == "true" ? true : false);
                config.setNotification_ext_connected(request->getParam(1)->value() == "true" ? true : false);
                config.setNotificationInputChange(request->getParam(2)->value() == "true" ? true : false);
                config.setNotification_manual_input(request->getParam(3)->value() == "true" ? true : false);
                config.saveEmailSettings();
                request->send(200, "text/plain", "OK");
              }
              else
              {
                request->send(400);
              } });

  server.on("/names", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->params() == 7)
              {
                Serial.printf("names save\n");
                auto module = request->getParam(0)->value().toInt();
                auto channelId = request->getParam(1)->value().toInt();
                auto name = request->getParam(2)->value();
                Serial.printf("module is %d\r\n", module);
                Serial.printf("ChannelID is %d\r\n", channelId);
                Serial.printf("Name is %s\r\n", name);
                  if (module == 0)
                  {
                    auto m = (VM208 *)mm.getBaseModule();
                    m->getChannel(channelId - 1)->setName(name);
                    Serial.println(m->getChannel(channelId-1)->getName());
                  }
                  else
                  {
                    auto m = (VM208EX *)mm.getModule(1);
                    m->getChannel(channelId - 1)->setName(name);
                    Serial.println(m->getChannel(channelId-1)->getName());
                  }
                config.setMosfet1Name(request->getParam(3)->value());
                config.setMosfet2Name(request->getParam(4)->value());
                config.setInputName(request->getParam(5)->value());
                config.setBoardName(request->getParam(6)->value());
                config.setName((module*4 + channelId), name);
                config.saveNames();
                config.save();
                request->send(200, "text/plain", "OK");
              }
              else
              {
                request->send(400);
              } });

  server.on("/time_settings", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              Serial.printf("time_settings\n");
              if (request->params() == 3)
              {
                Serial.println(request->getParam(0)->value().toInt());
                config.setTimezone(request->getParam(0)->value().toInt());
                Serial.println(request->getParam(1)->value().toInt());
                config.setDST(request->getParam(1)->value().toInt());
                configTime(config.getTimezone(), config.getDST(), "pool.ntp.org");
                config.setTimeZoneID(request->getParam(2)->value().toInt());
                Serial.println(request->getParam(2)->value().toInt());
                config.save();
                printLocalTime();
                //applyEthNetworkSettings();
                request->send(200, "text/plain", "OK");
              }
              else
              {
                request->send(400);
              } });

  server.on("/layout", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.printf("layout\n");
              //Generate layout in json and return it
              //StaticJsonBuffer<10000> json;
              DynamicJsonBuffer json;
              JsonObject &root = json.createObject();
              root["VM208"] = true;
              root["VM208EX"] = mm.isExtensionConnected();
              JsonArray &interfaces = root.createNestedArray("Interfaces");
              for (int i = 0; i < 8; i++)
              {
                auto size = mm.getAmountOfModulesOnInterface(i);
                if (size) //If modules available on this interface
                {
                  JsonObject &module = interfaces.createNestedObject();
                  for (int j = 0; j < 4; j++)
                  {
                    String available = "Socket";
                    available += String(j + 1);

                    if (mm.getModuleFromInterface(i, j) != nullptr)
                    {
                      module[available] = true;
                    }
                    else
                    {
                      module[available] = false;
                    }
                  }
                }
              }
              AsyncResponseStream *response = request->beginResponseStream("application/json");
              root.printTo(*response);
              request->send(response);
              json.clear(); });

  server.on("/shedule_set", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              Serial.printf("shedule_set\n");
              if (request->params() == 29)
              {
                uint16_t relay = request->getParam(0)->value().toInt() - 1;
                //VM208TimeChannel *c = (VM208TimeChannel *)getRelayChannelById(relay.toInt());
                bool state = true;
                String time;
                uint8_t hour;
                uint8_t minute;
                bool enabled;
                uint8_t dow;
                //Alarm *a;
                for (int i = 1; i < 29; i++)
                {
                  //Turn On Alarm
                  time = request->getParam(i)->value();  //get time
                  hour = time.substring(0, 2).toInt();   //split hour
                  minute = time.substring(3, 5).toInt(); // split minute

                  i++;
                  enabled = (request->getParam(i)->value() == "true") ? true : false; //get alarm enabled
                  state = i < 15;
                  if (i > 14)
                  {
                    dow = (i / 2) - 7;
                  }
                  else
                  {
                    dow = i / 2;
                  }
                  dow = dow == 7 ? 0 : dow;
                  /*Serial.println("SHEDULE: ");
      Serial.printf("Relay is: %d ",relay);
      Serial.printf("DOW is: %d ",dow);
      Serial.printf("HOUR is: %d ",hour);
      Serial.printf("Minute is: %d ",minute);
      Serial.printf("State is: %d ",state);
      Serial.printf("Enabled is %d ",enabled);*/
                  config.setShedule(relay, dow, hour, minute, state, enabled);
                  state = !state;
                }

                config.writeAlarm(relay + 1);
                sendSettings(request);
              }
              else
              {
                request->send(400);
              } });

  server.on(
      "/doUpdate", HTTP_POST, [](AsyncWebServerRequest *request) {},
      [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
      {
        handleDoUpdate(request, filename, index, data, len, final);
      });

  char user[32];
  char userpw[32];
  memset(user, 0, 32);
  memset(userpw, 0, 32);
  config.getUserName().toCharArray(user, 32);
  config.getUserPw().toCharArray(userpw, 32);
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html").setAuthentication(user, userpw).setFilter(ON_STA_VM208_FILTER);
  server.serveStatic("/", SPIFFS, "/ap/").setDefaultFile("index.html").setFilter(ON_AP_VM208_FILTER);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_VM208_FILTER);
  server.onNotFound([](AsyncWebServerRequest *request)
                    {
                      if (ON_AP_VM208_FILTER(nullptr))
                      {
                        //Send index.htm with default content type
                        request->send(SPIFFS, "/index.htm");
                      }
                      else
                      {
                        if (request->method() == HTTP_POST && request->url() == "index.html")
                        {
                          Serial.println("POST INDEX");
                          AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html");
                          request->send(response);
                        }
                        else
                        {
                          Serial.println(request->url());
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
                        }
                      } });
  server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                      {
                        if (!index)
                          Serial.printf("UploadStart: %s\n", filename.c_str());
                        Serial.printf("%s", (const char *)data);
                        if (final)
                          Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len); });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                       {
                         if (!index)
                           Serial.printf("BodyStart: %u\n", total);
                         Serial.printf("%s", (const char *)data);
                         if (index + len == total)
                           Serial.printf("BodyEnd: %u\n", total); });
  server.begin();
}

void handleDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
  int cmd = U_FLASH;
  if (!index)
  {
    log_i("Update");

    // if filename includes spiffs, update the spiffs partition

    cmd = (filename.startsWith("spiffs")) ? U_SPIFFS : U_FLASH;
    cmd ? Serial.println("SPIFFS") : Serial.println("FLASH");
    if (cmd == U_FLASH)
    {
      SPIFFS.end();
    }
    // disableIOacitivty();
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
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Please wait while the VM208 reboots");
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
      if (cmd == U_FLASH)
      {
        ESP.restart();
      }
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
  time_t now;
  struct tm *timeInfo;
  now = time(nullptr);
  timeInfo = localtime(&now);
  String hour = String((int)timeInfo->tm_hour, 10);
  String minute = String((int)timeInfo->tm_min, 10);
  String second = String((int)timeInfo->tm_sec, 10);
  String s;
  s += hour;
  s += ":";
  s += minute;
  s += ":";
  s += second;
  root.set("LOCAL_TIME", s);
  root.printTo(*response);
  request->send(response);
  jsonBuffer.clear();
}

void sendIOState(AsyncWebServerRequest *request)
{
  static pulseStatus_t pulseStatus;
  static pulseStatus_t timerStatus;
  xQueueReceive(pulseStatusQueue, &pulseStatus, 10 / portTICK_PERIOD_MS);
  xQueueReceive(timerStatusQueue, &timerStatus, 10 / portTICK_PERIOD_MS);
  // AsyncResponseStream *response = request->beginResponseStream("application/json");
  request->send(200);
  return;
  // const size_t capacity = JSON_OBJECT_SIZE(16) + 190;
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  // Input **inputs;
  // inputs = getCurrentInputs();
  // Mosfet *m1 = getMosfetById(1);
  // Mosfet *m2 = getMosfetById(2);
  VM208 *module = (VM208 *)mm.getModule(0);

  JsonObject &interface = root.createNestedObject("Interface0");
  JsonArray &channels = interface.createNestedArray("VM208");
  for (int j = 0; j < 4; j++)
  {
    xSemaphoreTake(g_Mutex, 1000 / portTICK_PERIOD_MS);
    VM208Channel *ch = module->getChannel(j);
    JsonObject &channel = channels.createNestedObject();
    channel.set("name", ch->getName());
    channel.set("state", ch->isOn());
    channel.set("pulseActive", pulseStatus.status[j]);
    channel.set("timerActive", timerStatus.status[j]);
    xSemaphoreGive(g_Mutex);
  }
  if (mm.getAmount() > 1)
  {
    if (mm.getModule(1)->hasSocket() == false) // native VM208EX
    {
      JsonArray &channels = interface.createNestedArray("VM208EX");
      for (int j = 0; j < 8; j++)
      {
        xSemaphoreTake(g_Mutex, 100 / portTICK_PERIOD_MS);
        VM208EXChannel ch = (*(VM208EX *)mm.getModule(1))[j];
        JsonObject &channel = channels.createNestedObject();
        channel.set("name", ch.getName());
        channel.set("state", ch.isOn());
        channel.set("pulseActive", pulseStatus.status[4 + j]);
        channel.set("timerActive", timerStatus.status[4 + j]);
        xSemaphoreGive(g_Mutex);
      }
    }
    else
    {
      /*JsonArray &interfaces = root.createNestedArray("Interfaces");
      for (int i = 0; i < 8; i++) //loop interfaces
      {
        auto modules = mm.getAmountOfModulesOnInterface(i);
        if (modules) //has interface a module?
        {
          JsonArray &interface = interfaces.createNestedArray();
          for (int x = 0; x < modules; x++) //loop over all modules
          {
            JsonArray &moduleObject = interface.createNestedArray();
            VM208EX *module = (VM208EX *)mm.getModuleFromInterface(i, x);
            for (int j = 0; j < 8; j++) //loop over all channels of a module
            {
              VM208EXChannel *ch = module->getChannel(j, false);
              JsonObject &channel = moduleObject.createNestedObject();
              channel["name"] = ch->getName();
              channel["state"] = ch->isOn();
              channel.set("pulseActive", pulseStatus.status[12 + (i * 32) + (x * 8) + j]);
              channel.set("timerActive", timerStatus.status[12 + (i * 32) + (x * 8) + j]);
            }
          }
        }
      }*/
    }
  }
  // root.set("input", false);
  // root.printTo(*response);
  // request->send(response);
  jsonBuffer.clear();
}

void sendIOState(AsyncWebServerRequest *request, int8_t interface, uint8_t socket)
{
  static pulseStatus_t pulseStatus;
  static pulseStatus_t timerStatus;
  xQueueReceive(pulseStatusQueue, &pulseStatus, 10 / portTICK_PERIOD_MS);
  xQueueReceive(timerStatusQueue, &timerStatus, 10 / portTICK_PERIOD_MS);
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  // const size_t capacity = JSON_OBJECT_SIZE(16) + 190;
  DynamicJsonBuffer jsonBuffer;
  // StaticJsonBuffer<20000> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  JsonArray &channels = root.createNestedArray("Channels");
  if (xSemaphoreTake(g_Mutex, 1000 / portTICK_PERIOD_MS))
  {
    if (interface == -1)
    {
      VM208 *module = mm.getBaseModule();
      for (int j = 0; j < 4; j++)
      {
        VM208Channel *ch = module->getChannel(j);
        JsonObject &channel = channels.createNestedObject();
        channel.set("name", ch->getName());
        channel.set("state", ch->isOn());
        channel.set("pulseActive", pulseStatus.status[j]);
        channel.set("timerActive", timerStatus.status[j]);
      }
    }
    else
    {
      if (interface == 0)
      {
        VM208EX *module = (VM208EX *)mm.getModule(1);
        for (int j = 0; j < 8; j++)
        {
          auto channelID = convertToChannelId(j + 1, interface, socket);
          VM208EXChannel *ch = module->getChannel(j, false);
          JsonObject &channel = channels.createNestedObject();
          channel.set("name", ch->getName());
          channel.set("state", ch->isOn());
          channel.set("pulseActive", pulseStatus.status[channelID - 1]);
          channel.set("timerActive", timerStatus.status[channelID - 1]);
        }
      }
      else
      {
        uint8_t moduleNr = 2 + ((interface - 1) * 4) + (socket - 1);
        VM208EX *module = (VM208EX *)mm.getModule(moduleNr);
        if (module != nullptr)
        {
          for (int j = 0; j < 8; j++)
          {
            auto channelID = convertToChannelId(j + 1, interface, socket);
            VM208EXChannel *ch = module->getChannel(j, false);
            JsonObject &channel = channels.createNestedObject();
            channel.set("name", ch->getName());
            channel.set("state", ch->isOn());
            channel.set("pulseActive", pulseStatus.status[channelID - 1]);
            channel.set("timerActive", timerStatus.status[channelID - 1]);
          }
        }
      }
    }
    root.set("m1", mm.getBaseModule()->getMosfet1State());
    root.set("m2", mm.getBaseModule()->getMosfet2State());
    root.set("input", mm.getBaseModule()->getUserInputState());
    xSemaphoreGive(g_Mutex);
  }
  root.printTo(*response);
  request->send(response);
  jsonBuffer.clear();
}

void sendSettings(AsyncWebServerRequest *request)
{

  // AsyncResponseStream *response = request->beginResponseStream("application/json");
  AsyncJsonResponse *response = new AsyncJsonResponse();
  // DynamicJsonBuffer jsonBuffer;
  JsonObject &root = response->getRoot();

  root.set(config.USERNAME_KEY, config.getUserName());
  root.set(config.BOARDNAME_KEY, config.getBoardName());
  root.set(config.SSID_KEY, config.getSSID());
  root.set(config.ETH_DHCPEN_KEY, config.getETH_DHCPEnable());
  root.set(config.ETH_IPADDR_KEY, config.getETH_IPAddress());          // TODO: change to interface settings
  root.set(config.ETH_GATEWAY_KEY, config.getETH_Gateway());           // TODO: change to interface settings
  root.set(config.ETH_SUBNETMASK_KEY, config.getETH_SubnetMask());     // TODO: change to interface settings
  root.set(config.ETH_PRIMARYDNS_KEY, config.getETH_PrimaryDNS());     // TODO: change to interface settings
  root.set(config.ETH_SECONDARYDNS_KEY, config.getETH_SecondaryDNS()); // TODO: change to interface settings
  root.set(config.WIFI_DHCPEN_KEY, config.getWIFI_DHCPEnable());
  root.set(config.WIFI_IPADDR_KEY, config.getWIFI_IPAddress());          // TODO: change to interface settings
  root.set(config.WIFI_GATEWAY_KEY, config.getWIFI_Gateway());           // TODO: change to interface settings
  root.set(config.WIFI_SUBNETMASK_KEY, config.getWIFI_SubnetMask());     // TODO: change to interface settings
  root.set(config.WIFI_PRIMARYDNS_KEY, config.getWIFI_PrimaryDNS());     // TODO: change to interface settings
  root.set(config.WIFI_SECONDARYDNS_KEY, config.getWIFI_SecondaryDNS()); // TODO: change to interface settings
  root.set(config.TIMEZONEID_KEY, config.getTimeZoneID());
  root.set(config.TIMEZONE_KEY, config.getTimezone());
  root.set(config.DST_KEY, config.getDST());
  root.set(config.NAME_INPUT_KEY, config.getInputName());
  root.set(config.NAME_MOSFET1_KEY, config.getMosfet1Name());
  root.set(config.NAME_MOSFET2_KEY, config.getMosfet2Name());
  root.set(config.NOTIF_BOOT_KEY, config.getNotificationBoot());
  root.set(config.NOTIF_EXT_CONNECT_KEY, config.getNotification_ext_connected());
  root.set(config.NOTIF_INPUT_CHANGE_KEY, config.getNotificationInputChange());
  root.set(config.NOTIF_MANUAL_INPUT_KEY, config.getNotification_manual_input());
  root.set("smtpserver", config.getEmailServer());
  root.set("smtpport", config.getEmailPort());
  root.set("username", config.getEmailUser());
  root.set("recipient", config.getEmailRecipient());
  root.set("subject", config.getEmailSubject());
  /*JsonArray &Channels = root.createNestedArray("Channels");
  VM208TimeChannel *c;
  Alarm *a;

  for (int i = 0; i < 12; i++)
  {
    c = (VM208TimeChannel *)getRelayChannelById(i + 1);
    JsonObject &Channel = Channels.createNestedObject();
    Channel[config.CHANNEL_NAME_KEY] = c->getName();
    JsonArray &Channels_alarms = Channel.createNestedArray("alarms");
    for (int j = 0; j < 14; j++)
    {
      a = c->getAlarm(j);
      JsonObject &Channels_alarms_settings = Channels_alarms.createNestedObject();
      Channels_alarms_settings[config.ALARM_WEEKDAY_KEY] = a->getWeekday();
      Channels_alarms_settings[config.ALARM_HOUR_KEY] = a->getHour();
      Channels_alarms_settings[config.ALARM_MINUTE_KEY] = a->getMinute();
      Channels_alarms_settings[config.ALARM_STATE_KEY] = a->getState();
      Channels_alarms_settings[config.ALARM_ENABLED_KEY] = a->isEnabled();
    }
  }*/
  response->setLength();
  request->send(response);
}

bool ON_AP_VM208_FILTER(AsyncWebServerRequest *request)
{
  wifi_mode_t mode = WiFi.getMode();
  if ((mode == WIFI_MODE_AP) || (mode == WIFI_MODE_APSTA))
  {
    Serial.println("AP-Filter: true");
    return true;
  }
  else
  {
    Serial.println("AP-Filter: false");
    return false;
  }
}

bool ON_STA_VM208_FILTER(AsyncWebServerRequest *request)
{

  wifi_mode_t mode = WiFi.getMode();
  if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
  {
    Serial.println("STA-Filter: false");
    return false;
  }
  else
  {
    Serial.println("STA-Filter: true");
    return true;
  }
}

uint16_t convertToChannelId(uint8_t channel, int8_t interface, int8_t socket)
{
  uint16_t channelID = 0;
  if (interface == -1)
  {
    channelID = channel;
  }
  else if (interface == 0)
  {
    channelID = channel + 4;
  }
  else
  {
    channelID = 12 + ((interface - 1) * 32) + ((socket - 1) * 8) + channel;
  }
  return channelID;
}
// #endregion server