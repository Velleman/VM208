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
#include "VM208TimerChannel.hpp"

const char *TAG = "SERVER";

// SKETCH BEGIN
AsyncWebServer server(80);

void sendBoardInfo(AsyncWebServerRequest *request);
void sendIOState(AsyncWebServerRequest *request);
//String getMacAsString(uint8_t *mac);

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    //request->addInterestingHeader("ANY");
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
      ESP.restart();
    }
  });

  server.on("/relay", HTTP_POST, [](AsyncWebServerRequest *request) {
    String relay;
    String state;
    AsyncWebParameter *p = request->getParam(0);
    relay = p->value();
    p = request->getParam(1);
    state = p->value();
    
    RelayChannel* channel = mm.getChannel(relay.toInt());
    state.toInt() ? channel->turnOn() : channel->turnOff();

    sendIOState(request);
  });

  server.on("/mosfet", HTTP_POST, [](AsyncWebServerRequest *request) {
    String relay;
    String state;
    AsyncWebParameter *p = request->getParam(0);
    relay = p->value();
    p = request->getParam(1);
    state = p->value();
    /*Mosfet *m = getMosfetById(relay.toInt());
    if (state == "0")
    {
      m->turnOff();
    }
    else
    {
      m->turnOn();
    }*/

    sendIOState(request);
  });

  server.on("/pulse", HTTP_POST, [](AsyncWebServerRequest *request) {
    String relay;
    String time;
    AsyncWebParameter *p = request->getParam(0);
    relay = p->value();
    p = request->getParam(1);
    time = p->value();
    VM208TimeChannel *c = (VM208TimeChannel *)getRelayChannelById(relay.toInt());
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
    VM208TimeChannel *c = (VM208TimeChannel *)getRelayChannelById(relay.toInt());
    c->activateTimer(time.toInt());
    sendIOState(request);
  });

  server.on("/email_settings", HTTP_POST, [](AsyncWebServerRequest *request) {
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
    }
  });

  server.on("/testmail", HTTP_POST, [](AsyncWebServerRequest *request) {
    sendTestMail();
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
      Serial.printf("%s", config.getSSID());
      Serial.printf("%s", config.getWifiPassword());
      Serial.printf("Wifi Saved\n");
      ESP.restart();
    }
    else
    {
      request->send(400);
    }
  });

  server.on("/wifi_creds_save", HTTP_POST, [](AsyncWebServerRequest *request) {
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
      applyEthNetworkSettings();
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
      applyWifiNetworkSettings();
      request->send(200);
    }
    else
    {
      request->send(400);
    }
  });

  server.on("/notif_setting", HTTP_POST, [](AsyncWebServerRequest *request) {
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
      VM208TimeChannel *c = (VM208TimeChannel *)getRelayChannelById(relais);

      Alarm *a = c->getAlarm((day * 2) + state);

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

  server.on("/names", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->params() == 16)
    {
      Serial.printf("names save\n");
      VM208TimeChannel *c;
      for (int i = 0; i < 12; i++)
      {
        c = (VM208TimeChannel *)getRelayChannelById(i + 1);
        c->setName(request->getParam(i)->value());
      }
      config.setMosfet1Name(request->getParam(12)->value());
      config.setMosfet2Name(request->getParam(13)->value());
      config.setInputName(request->getParam(14)->value());
      config.setBoardName(request->getParam(15)->value());
      config.saveAlarms();
      config.save();
      request->send(200, "text/plain", "OK");
    }
    else
    {
      request->send(400);
    }
  });

  server.on("/time_settings", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.printf("time_settings\n");
    if (request->params() == 2)
    {

      config.setTimezone(request->getParam(0)->value().toInt());
      config.setDST(request->getParam(1)->value().toInt());
      configTime(config.getTimezone(), config.getDST(), "pool.ntp.org");
      config.save();
      //applyEthNetworkSettings();
      request->send(200, "text/plain", "OK");
    }
    else
    {
      request->send(400);
    }
  });

  server.on("/shedule_set", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.printf("shedule_set\n");
    if (request->params() == 29)
    {
      String relay = request->getParam(0)->value();
      VM208TimeChannel *c = (VM208TimeChannel *)getRelayChannelById(relay.toInt());
      bool state = true;
      int param = 1;
      String time;
      String hour;
      String minute;
      bool enabled;
      uint8_t alarm = 0;
      Alarm *a;
      for (int i = 1; i < 7; i++)
      { //do monday to saturday 1-6 Sunday is 0 handle out of loop
        //Turn On Alarm
        time = request->getParam(param)->value();                               //get time
        hour = time.substring(0, 2);                                            //split hour
        minute = time.substring(3, 5);                                          // split minute
        param++;                                                                //increase param;
        enabled = (request->getParam(param)->value() == "true") ? true : false; //get alarm enabled

        a = c->getAlarm(alarm);
        a->setHour(hour.toInt());
        a->setMinute(minute.toInt());
        a->setEnabled(enabled);
        state = !state;
        alarm++;
        //Turn Off Alarm
        param++;
        time = request->getParam(param)->value();
        hour = time.substring(0, 2);
        minute = time.substring(3, 5);
        param++;
        enabled = (request->getParam(param)->value() == "true") ? true : false; //get alarm enabled

        a = c->getAlarm(alarm);
        a->setHour(hour.toInt());
        a->setMinute(minute.toInt());
        a->setEnabled(enabled);
        state = !state;
        alarm++;
        param++;
      }
      //Ugly ass bitch code....
      //Turn On Alarm
      time = request->getParam(param)->value();                               //get time
      hour = time.substring(0, 2);                                            //split hour
      minute = time.substring(3, 5);                                          // split minute
      param++;                                                                //increase param;
      enabled = (request->getParam(param)->value() == "true") ? true : false; //get alarm enabled

      a = c->getAlarm(alarm);
      a->setHour(hour.toInt());
      a->setMinute(minute.toInt());
      a->setEnabled(enabled);
      state = !state;
      alarm++;
      //Turn Off Alarm
      param++;
      time = request->getParam(param)->value();
      hour = time.substring(0, 2);
      minute = time.substring(3, 5);
      param++;
      enabled = (request->getParam(param)->value() == "true") ? true : false; //get alarm enabled

      a = c->getAlarm(alarm);
      a->setHour(hour.toInt());
      a->setMinute(minute.toInt());
      a->setEnabled(enabled);
      state = !state;

      config.saveAlarms();
      sendSettings(request);
    }
    else
    {
      request->send(400);
    }
  });

  server.on(
      "/doUpdate", HTTP_POST, [](AsyncWebServerRequest *request) {},
      [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
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
  server.onNotFound([](AsyncWebServerRequest *request) {
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
    }
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
    //disableIOacitivty();
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

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  //const size_t capacity = JSON_OBJECT_SIZE(16) + 190;
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  //Input **inputs;
  //inputs = getCurrentInputs();
  //Mosfet *m1 = getMosfetById(1);
  //Mosfet *m2 = getMosfetById(2);
  VM208 *module = mm.getBaseModule();

  JsonObject &interface = root.createNestedObject("Interface0");
  JsonArray &channels = interface.createNestedArray("VM208");
  for (int j = 0; j < 4; j++)
  {
    VM208Channel ch = (*module)[j];
    JsonObject &channel = channels.createNestedObject();
    channel.set("name", ch.getName());
    channel.set("state", ch.isOn());
  }
  if (mm.getAmount() > 1)
  {
    if (mm.getModule(1)->hasSocket() == false) //native VM208EX
    {
      JsonArray &channels = interface.createNestedArray("VM208EX");
      for (int j = 0; j < 8; j++)
      {
        VM208EXChannel ch = (*(VM208EX *)mm.getModule(1))[j];
        JsonObject &channel = channels.createNestedObject();
        channel.set("name", ch.getName());
        channel.set("state", ch.isOn());
      }
    }
    else
    {
      JsonArray& interfaces = root.createNestedArray("Interfaces");
      for (int i = 0; i < 8; i++)//loop interfaces
      {
        auto modules = mm.getAmountOfModulesOnInterface(i);
        if (modules)//has interface a module?
        {
          JsonArray& interface = interfaces.createNestedArray();
          for (int x = 0; x < modules; x++)//loop over all modules
          {
            JsonArray& moduleObject = interface.createNestedArray();
            VM208EX *module = (VM208EX *)mm.getModuleFromInterface(i,x);
            for (int j = 0; j < 8; j++)//loop over all channels of a module
            {
              VM208EXChannel* ch = module->getChannel(j);
              JsonObject &channel = moduleObject.createNestedObject();
              channel["name"] = ch->getName();
              channel["state"] = ch->isOn();
            }
          }
        }
      }
    }
  }
  root.set("input", false);
  root.printTo(*response);
  request->send(response);
  jsonBuffer.clear();
}

void sendSettings(AsyncWebServerRequest *request)
{

  //AsyncResponseStream *response = request->beginResponseStream("application/json");
  AsyncJsonResponse *response = new AsyncJsonResponse();
  //DynamicJsonBuffer jsonBuffer;
  JsonObject &root = response->getRoot();

  root.set(config.USERNAME_KEY, config.getUserName());
  root.set(config.BOARDNAME_KEY, config.getBoardName());
  root.set(config.SSID_KEY, config.getSSID());
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
    return true;
  }
}
//#endregion server