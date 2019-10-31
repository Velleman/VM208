#include "global.hpp"
#include <Arduino.h>

#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>

#include "server_VM208.h"
#include "IO.hpp"
#include "AsyncUDP.h"
#include <SPIFFS.h>
#include "config_vm208.hpp"

#include "input.hpp"
#include "output.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <DNSServer.h>
#include "mail.hpp"
#include "network_VM208.hpp"
#include "time_VM208.hpp"
#include <DNSServer.h>
//static const char *TAG = "VM208_MAIN";

AsyncUDP udp;
// Define NTP Client to get time
WiFiUDP ntpUDP;

SemaphoreHandle_t g_Mutex;
SemaphoreHandle_t g_MutexChannel;
SemaphoreHandle_t g_MutexMail;
EventGroupHandle_t s_wifi_event_group;
DNSServer dnsServer;
int8_t timeZone = 1;
int8_t minutesTimeZone = 0;
const PROGMEM char *ntpServer = "pool.ntp.org";
bool wifiFirstConnected = false;
//DNSServer dnsServer;
Configuration config;
bool gotETH_IP;
bool gotSTA_IP;

String getMacAsString(uint8_t *mac);

static void checkSheduler(void *pvParameter)
{
  delay(500); // wait to load all channels
  //uint8_t *id = (uint8_t *)pvParameter;
  Channel *c;
  Alarm *alarmOn;
  Alarm *alarmOff;
  uint8_t day;
  time_t t;
  struct tm *current_time;
  while (true)
  {
    t = time(NULL);
    current_time = localtime(&t);
    day = current_time->tm_wday;
    day = day == 0 ? 6 : (day - 1);

    for (int i = 0; i < 12; i++)
    {
      c = getChannelById(i + 1);
      if (c->isSheduleActive())
      {
        alarmOn = c->getAlarm(day * 2);
        alarmOff = c->getAlarm((day * 2) + 1);
        if (alarmOn->isEnabled())
        {
          if (current_time->tm_hour == alarmOn->getHour() && current_time->tm_min == alarmOn->getMinute())
          {
            if (c->getState() != alarmOn->getState())
            {
              //ESP_LOGI(TAG, "Alarm triggered");
              alarmOn->getState() ? c->turnOn() : c->turnOff();
            }
          }
        }
        if (alarmOff->isEnabled())
        {
          if (current_time->tm_hour == alarmOff->getHour() && current_time->tm_min == alarmOff->getMinute())
          {
            if (c->getState() != alarmOff->getState())
            {
              //ESP_LOGI(TAG, "Alarm triggered");
              alarmOff->getState() ? c->turnOn() : c->turnOff();
            }
          }
        }
      }
    }
    delay(500);
  }
  vTaskDelete(NULL);
}

static void shedulerStatus(void *pvParameter)
{
  delay(500); // wait to load all channels
  Channel *c;
  while (true)
  {
    for (int i = 0; i < 12; i++)
    {
      c = getChannelById(i + 1);
      if (c->isSheduleActive())
      {
        c->setLed(c->getState());
      }
    }
    delay(1800);
    for (int i = 0; i < 12; i++)
    {
      c = getChannelById(i + 1);
      if (c->isSheduleActive())
      {
        c->toggleLed();
      }
    }
    delay(200);
  }
  vTaskDelete(NULL);
}

void setup()
{
  g_Mutex = xSemaphoreCreateMutex();
  g_MutexChannel = xSemaphoreCreateMutex();
  g_MutexMail = xSemaphoreCreateMutex();
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  esp_reset_reason_t reason = esp_reset_reason();
  bool setState = true;
  if (reason == ESP_RST_TASK_WDT || reason == ESP_RST_PANIC || reason == ESP_RST_UNKNOWN)
  {
    Serial.println("reset because of WDT");
    setState = false;
  }
  delay(10);
  Serial.printf("START\n");

  ESP_LOGI(TAG, "APP MAIN ENTRY");
  SPIFFS.begin();
  Init_IO(setState);

  s_wifi_event_group = xEventGroupCreate();

  //ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  config.load();

  Input **inputs;
  inputs = getCurrentInputs();
  readInputs(inputs);

  //check if button 1 and 4 is pressed
  //start AP for WiFi Config
  ESP_LOGI(TAG, "Check Buttons");
  if (((inputs[0]->read() == false) && (inputs[3]->read() == false)) || config.getFirstTime())
  {
    startEth();
    ESP_LOGI(TAG, "Start AP");
    WiFi.softAP("VM208_AP", "VellemanForMakers");
    WiFi.enableSTA(false);
    IPAddress apIP(192, 168, 4, 1);

    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  else
  {
    xTaskCreate(IO_task, "IO_task", 3072, NULL, (tskIDLE_PRIORITY + 2), NULL);
    WiFi.onEvent(WiFiEvent);
    if (!startEth()) //No cable inserted
    {
      startWifi();
    }

    xTaskCreate(got_ip_task, "got_ip_task", 4096, NULL, (tskIDLE_PRIORITY + 2), NULL);
    xTaskCreate(time_keeping_task, "time_keeping", 1024, NULL, (tskIDLE_PRIORITY + 2), NULL);

    //#region hide
    if (udp.listen(30303))
    {
      
      udp.onPacket([](AsyncUDPPacket packet) {
        Serial.print("UDP Packet Type: ");
        Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
        Serial.print(", From: ");
        Serial.print(packet.remoteIP());
        Serial.print(":");
        Serial.print(packet.remotePort());
        Serial.print(", To: ");
        Serial.print(packet.localIP());
        Serial.print(":");
        Serial.print(packet.localPort());
        Serial.print(", Length: ");
        Serial.print(packet.length());
        Serial.print(", Data: ");
        Serial.write(packet.data(), packet.length());
        Serial.println();
        //reply to the client
        String message = "Aloha, My Name is:\n";
        message += config.getBoardName();
        message += "\n";
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_ETH);
        message += getMacAsString(mac);
        packet.print(message);
      });
    }
    if (!MDNS.begin("VM208"))
    {
      ESP_LOGI(TAG, "Error setting up MDNS responder!");
    }
    else
    {
      //#endregion hide
      MDNS.addService("http", "tcp", 80);
    }
  }
  xTaskCreate(checkSheduler, "Sheduler", 8192, NULL, (tskIDLE_PRIORITY + 2), NULL);
  xTaskCreate(shedulerStatus, "ShedulerStatus", 8192, NULL, (tskIDLE_PRIORITY + 2), NULL);
  startServer();
  if(WiFi.getMode() != WIFI_MODE_AP)
    sendBootMail();
}

void loop()
{
  if(WiFi.getMode() == WIFI_MODE_AP)
  {
    dnsServer.processNextRequest();
  }
  else
  {
    ArduinoOTA.handle();
  }
}

