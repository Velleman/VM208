#include "global.hpp"
#include <Arduino.h>
//#include <Hash.h>
//#include <ESP8266WiFi.h>
#include <WiFi.h>
//#include <ESP8266mDNS.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
// we need to store long long
//#include <ESPAsyncTCP.h>
#include <AsyncTCP.h>

#include "server_VM208.h"

#include "IO.hpp"

#include "tcpip_adapter.h"
#include "soc/emac_ex_reg.h"
#include "driver/periph_ctrl.h"
#include "WifiConfig.h"

#include "AsyncUDP.h"

#include "eth_phy/phy_lan8720.h"
#define DEFAULT_ETHERNET_PHY_CONFIG phy_lan8720_default_ethernet_config
#include <SPIFFS.h>
#include "config_vm208.hpp"

#include "input.hpp"
#include "output.hpp"
#include "esp_log.h"
#include "FreeRTOS.h"
#include <ETH.h>
#include <DNSServer.h>
#include "mail.hpp"
//static const char *TAG = "VM208_MAIN";

AsyncUDP udp;
// Define NTP Client to get time
WiFiUDP ntpUDP;
Configuration config;
bool gotETH_IP;
bool gotSTA_IP;

#define PIN_PHY_POWER GPIO_NUM_5
#define PIN_SMI_MDC 23
#define PIN_SMI_MDIO 18

SemaphoreHandle_t g_Mutex;
SemaphoreHandle_t g_MutexChannel;
SemaphoreHandle_t g_MutexMail;


int8_t timeZone = 1;
int8_t minutesTimeZone = 0;
const PROGMEM char *ntpServer = "pool.ntp.org";
bool wifiFirstConnected = false;
//DNSServer dnsServer;
void startWifi();

/**
* @brief event handler for ethernet
*
* @param ctx
* @param event
* @return esp_err_t
*/
void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_ETH_CONNECTED:
    WiFi.mode(WIFI_OFF);
    ESP_LOGI(TAG, "Ethernet Link Up");
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    gotETH_IP = false;
    startWifi();
    ESP_LOGI(TAG, "Ethernet Link Down");
    break;
  case SYSTEM_EVENT_ETH_START:
    ESP_LOGI(TAG, "Ethernet Started");
    ETH.setHostname("VM208");
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    /*/memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
    ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip));
    ESP_LOGI(TAG, "Ethernet Got IP Addr");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip.ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip.netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip.gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");*/
    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());
    if (ETH.fullDuplex())
    {
      Serial.print(", FULL_DUPLEX");
    }
    Serial.print(", ");
    Serial.print(ETH.linkSpeed());
    Serial.println("Mbps");
    gotETH_IP = true;
    xEventGroupSetBits(s_wifi_event_group, GOTIP_BIT);
    break;
  case SYSTEM_EVENT_ETH_STOP:
    ESP_LOGI(TAG, "Ethernet Stopped");
    break;
  case SYSTEM_EVENT_STA_START:
    tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, "VM208");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    gotSTA_IP = true;
    ESP_LOGI(TAG, "GOT_STA_IP");
    ESP_LOGI(TAG, "got ip:%s", WiFi.localIP());
    xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    xEventGroupSetBits(s_wifi_event_group, GOTIP_BIT);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT | GOTIP_BIT);
    gotSTA_IP = false;
    break;
  default:
    break;
  }
  //return ESP_OK;
}

static void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

static void got_ip_task(void *pvParameter)
{
  //tcpip_adapter_ip_info_t ip;
  //esp_err_t err;
  while (true)
  {
    xEventGroupWaitBits(s_wifi_event_group, GOTIP_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    if (gotETH_IP)
    {
      //esp_wifi_stop();
      WiFi.mode(WIFI_OFF);
    }
    const char *ntpServer = "pool.ntp.org";
    long gmtOffset_sec = config.getTimezone();
    int daylightOffset_sec = config.getDST();
    Serial.print("GMT OFFSET: ");
    Serial.println(gmtOffset_sec);
    Serial.print("DST OFFSET: ");
    Serial.println(daylightOffset_sec);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //sendEmail();
  }
}

static void time_keeping_task(void *pvParameter)
{
  while (true)
  {
    time_t now;
    struct tm *timeInfo;
    now = time(nullptr);
    timeInfo = localtime(&now);
    if (timeInfo->tm_hour == 3 && timeInfo->tm_min == 5)
    {
      Serial.println("Daily Time Check");
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo))
      {
        Serial.println("Failed to obtain time");
        return;
      }
    }
    delay(500);
  }
  vTaskDelete(NULL);
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

void startWifi()
{

  if (config.getSSID().equals("") || config.getWifiPassword().equals(""))
  {
    ESP_LOGI(TAG, "INVALID WIFI CREDENTIALS");
  }
  else
  {
    if (!config.getWIFI_DHCPEnable())
    {
      char ip[15];
      char gw[15];
      char sub[15];
      char prim[15];
      char sec[15];
      config.getWIFI_IPAddress().toCharArray(ip, 15);
      config.getWIFI_Gateway().toCharArray(gw, 15);
      config.getWIFI_SubnetMask().toCharArray(sub, 15);
      config.getWIFI_PrimaryDNS().toCharArray(prim, 15);
      config.getWIFI_SecondaryDNS().toCharArray(sec, 15);

      IPAddress local_IP(ipaddr_addr(ip));
      IPAddress gateway(ipaddr_addr(gw));
      IPAddress subnet(ipaddr_addr(sub));
      IPAddress primaryDNS(ipaddr_addr(prim));
      IPAddress secondaryDNS(ipaddr_addr(sec));

      if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
      {
        Serial.println("STA Failed to configure");
      }
    }
    char ssid[15];
    char pw[15];
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
    config.getSSID().toCharArray(ssid, 15);
    config.getWifiPassword().toCharArray(pw, 15);
    Serial.println("Connecting to:");
    Serial.println(config.getSSID());
    Serial.println(config.getWifiPassword());

    //WiFi.disconnect(false,false);

    WiFi.begin(ssid, pw);
    while (!WiFi.isConnected())
    {
      Serial.print(".");
      delay(200);
    }
  }
}
/*static void applyEthNetworkSettings()
{
  if (!config.getETH_DHCPEnable())
  {
    Serial.println("ETH DHCP DISABLED");
    Serial.println(config.getETH_IPAddress());
    Serial.println(config.getETH_Gateway());
    Serial.println(config.getETH_SubnetMask());
    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);
    //static ip
    tcpip_adapter_ip_info_t info;
    char ip[15];
    char gw[15];
    char subnet[15];
    config.getETH_IPAddress().toCharArray(ip, 15);
    config.getETH_Gateway().toCharArray(gw, 15);
    config.getETH_SubnetMask().toCharArray(subnet, 15);
    info.ip.addr = ipaddr_addr(ip);
    info.gw.addr = ipaddr_addr(gw);
    info.netmask.addr = ipaddr_addr(subnet);
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &info);
    //set primnary DNS
    tcpip_adapter_dns_info_t info_dns;
    char dns[15];
    config.getETH_PrimaryDNS().toCharArray(dns, 15);
    info_dns.ip.type = IPADDR_TYPE_V4;
    info_dns.ip.u_addr.ip4.addr = ipaddr_addr(dns);
    tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_ETH, TCPIP_ADAPTER_DNS_MAIN, &info_dns);
    //set secondary DNS
    config.getETH_SecondaryDNS().toCharArray(dns, 15);
    info_dns.ip.type = IPADDR_TYPE_V4;
    info_dns.ip.u_addr.ip4.addr = ipaddr_addr(dns);
    tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_ETH, TCPIP_ADAPTER_DNS_BACKUP, &info_dns);
  }
  else
  {
    Serial.println("ETH DHCP ENABLED");
    tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_ETH);
  }
}
*/
static bool startEth()
{
  // esp_err_t ret;
  // applyEthNetworkSettings();
  // eth_config_t eth_config = DEFAULT_ETHERNET_PHY_CONFIG;
  // /* Set the PHY address in the example configuration */
  // eth_config.phy_addr = PHY0;
  // eth_config.gpio_config = eth_gpio_config_rmii;
  // eth_config.tcpip_input = tcpip_adapter_eth_input;
  // eth_config.clock_mode = ETH_CLOCK_GPIO0_IN;
  // eth_config.phy_power_enable = phy_device_power_enable_via_gpio;
  // ret = esp_eth_init(&eth_config);

  // if (ret == ESP_OK)
  // {
  //   esp_eth_enable();
  // }

  pinMode(GPIO_NUM_2, OUTPUT);
  digitalWrite(GPIO_NUM_2, HIGH);
  ETH.begin(0, PIN_PHY_POWER, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
  if (!config.getETH_DHCPEnable())
  {
    char ip[15];
    char gw[15];
    char sub[15];
    char prim[15];
    char sec[15];
    config.getETH_IPAddress().toCharArray(ip, 15);
    config.getETH_Gateway().toCharArray(gw, 15);
    config.getETH_SubnetMask().toCharArray(sub, 15);
    config.getETH_PrimaryDNS().toCharArray(prim, 15);
    config.getETH_SecondaryDNS().toCharArray(sec, 15);

    IPAddress local_IP(ipaddr_addr(ip));
    IPAddress gateway(ipaddr_addr(gw));
    IPAddress subnet(ipaddr_addr(sub));
    IPAddress primaryDNS(ipaddr_addr(prim));
    IPAddress secondaryDNS(ipaddr_addr(sec));

    ETH.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  }
  
  return ETH.linkUp();
}

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
  //Serial.setDebugOutput(true);
  delay(10);
  Serial.printf("START\n");

  //ESP_ERROR_CHECK(nvs_flash_init());
  ESP_LOGI(TAG, "APP MAIN ENTRY");
  SPIFFS.begin();
  Init_IO();

  //tcpip_adapter_init();
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

    ESP_LOGI(TAG, "Start AP");
    WiFi.softAP("VM208_AP", "VellemanForMakers");
    WiFi.enableSTA(false);
    IPAddress apIP(192, 168, 4, 1);

    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    //dnsServer.start(53, "*", apIP);
  }
  else
  {
    xTaskCreate(IO_task, "IO_task", 3072, NULL, (tskIDLE_PRIORITY + 2), NULL);
    WiFi.onEvent(WiFiEvent);
    if(!startEth())//No cable inserted
    {
      startWifi();
    }

    xTaskCreate(got_ip_task, "got_ip_task", 4096, NULL, (tskIDLE_PRIORITY + 2), NULL);
    //xEventGroupWaitBits(s_wifi_event_group, GOTIP_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
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
        char boardname[16];
        config.getBoardName().toCharArray(boardname, 16, 0);
        packet.printf("Aloha, My Name is:%s", boardname);
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
  //sendBootMail();
}

void loop()
{
  ArduinoOTA.handle();
  delay(100);
  //Serial.println(esp_get_free_heap_size());
}
