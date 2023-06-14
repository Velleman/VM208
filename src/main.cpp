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

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <DNSServer.h>
#include "mail.hpp"
#include "network_VM208.hpp"
#include "time_VM208.hpp"
#include <DNSServer.h>
static const char *TAG = "VM208_MAIN";

AsyncUDP udp;
// Define NTP Client to get time
WiFiUDP ntpUDP;

SemaphoreHandle_t g_Mutex = NULL;
SemaphoreHandle_t g_MutexChannel = NULL;
SemaphoreHandle_t g_MutexMail;
DNSServer dnsServer;
int8_t timeZone = 1;
int8_t minutesTimeZone = 0;
const PROGMEM char *ntpServer = "pool.ntp.org";
bool wifiFirstConnected = false;
// DNSServer dnsServer;
Configuration config;
bool gotETH_IP;
bool gotSTA_IP;
ModuleManager mm;
String getMacAsString(uint8_t *mac);
EventGroupHandle_t s_wifi_event_group;
static void checkSheduler(void *pvParameter)
{
  delay(500); // wait to load all channels
  // uint8_t *id = (uint8_t *)pvParameter;
  uint8_t day;
  time_t t;
  struct tm *current_time;
  while (true)
  {
    t = time(NULL);
    current_time = localtime(&t);
    day = current_time->tm_wday;
    day = day == 0 ? 6 : (day - 1);

    for (int i = 0; i < 268; i++)
    {
      ChannelShedule *cs = config.getShedule(i);
      if (cs != nullptr)
        cs->Update(current_time);
    }

    delay(500);
  }
  vTaskDelete(NULL);
}
TaskHandle_t taskHandle;
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
  config.load();
  Init_IO(setState);

  s_wifi_event_group = xEventGroupCreate();

  // ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

  // check if button 1 and 4 is pressed
  // start AP for WiFi Config
  WiFi.onEvent(WiFiEvent);
  ESP_LOGI(TAG, "Check Buttons");
  if(xSemaphoreTake(g_Mutex,1000/portTICK_PERIOD_MS)){
  if (((mm.getBaseModule()->getChannel(0)->isButtonPressed()) && (mm.getBaseModule()->getChannel(3)->isButtonPressed())) || config.getFirstTime())
  {
    Serial.print("It's the first time :");
    mm.getBaseModule()->getChannel(0)->turnLedOn();
    mm.getBaseModule()->getChannel(3)->turnLedOn();
    delay(500);
    mm.getBaseModule()->getChannel(0)->turnLedOff();
    mm.getBaseModule()->getChannel(3)->turnLedOff();
    delay(500);
    mm.getBaseModule()->getChannel(0)->turnLedOn();
    mm.getBaseModule()->getChannel(3)->turnLedOn();
    delay(500);
    mm.getBaseModule()->getChannel(0)->turnLedOff();
    mm.getBaseModule()->getChannel(3)->turnLedOff();
    xSemaphoreGive(g_Mutex);
    ESP_LOGI(TAG, "Start AP");
    Serial.print("Start AP");
    WiFi.softAP("VM208_AP", "VellemanForMakers");
    WiFi.enableSTA(false);
    IPAddress apIP(192, 168, 4, 1);

    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  else
  {
    xSemaphoreGive(g_Mutex);
    Serial.printf("%s", config.getSSID().c_str());
    Serial.printf("%s", config.getWifiPassword().c_str());

    
    if (!startEth()) // No cable inserted
    {
      startWifi();
    }
  }
  }
  

  if (WiFi.getMode() != WIFI_MODE_AP)
  {
    xTaskCreate(IO_task, "io_task", 4096, NULL, (tskIDLE_PRIORITY + 3), &taskHandle);
  }

  xTaskCreate(got_ip_task, "got_ip_task", 4096, NULL, (tskIDLE_PRIORITY + 2), NULL);
  // xTaskCreate(time_keeping_task, "time_keeping", 1024, NULL, (tskIDLE_PRIORITY + 2), NULL);

  //#region hide
  if (udp.listen(30303))
  {

    udp.onPacket([](AsyncUDPPacket packet)
                 {
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
      packet.print(message); });
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
  //}
  if (WiFi.getMode() != WIFI_MODE_AP)
  {
    xTaskCreate(checkSheduler, "Sheduler", 8192, NULL, (tskIDLE_PRIORITY + 2), NULL);
  }

  startServer();
  if (WiFi.getMode() != WIFI_MODE_AP)
  {
    sendBootMail();
  }
}

void loop()
{
  if (WiFi.getMode() == WIFI_MODE_AP)
  {
    //Serial.println("AP Loop");
    dnsServer.processNextRequest();
    // vTaskDelay(10/portTICK_PERIOD_MS);

  }
  else
  {
    ArduinoOTA.handle();
  }
}
