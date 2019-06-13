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
#include "configuration.hpp"

#include "input.hpp"
#include "output.hpp"
#include "esp_log.h"
static const char *TAG = "VM208_MAIN";

AsyncUDP udp;
// Define NTP Client to get time
WiFiUDP ntpUDP;
Configuration config;
bool gotETH_IP;
bool gotSTA_IP;
void startWifi();

#define PIN_PHY_POWER GPIO_NUM_5
#define PIN_SMI_MDC 23
#define PIN_SMI_MDIO 18

SemaphoreHandle_t g_Mutex;

int8_t timeZone = 1;
int8_t minutesTimeZone = 0;
const PROGMEM char *ntpServer = "pool.ntp.org";
bool wifiFirstConnected = false;

static void phy_device_power_enable_via_gpio(bool enable)
{
  ESP_LOGI(TAG, "PHY USE POWER PIN ENABLED");
  assert(DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable);

  if (!enable)
  {
    /* Do the PHY-specific power_enable(false) function before powering down */
    DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable(false);
  }
  if (enable)
  {
    gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_2, 1);
  }
  ESP_LOGI(TAG, "power pin:%d", PIN_PHY_POWER);
  gpio_pad_select_gpio(PIN_PHY_POWER);
  gpio_set_direction(PIN_PHY_POWER, GPIO_MODE_OUTPUT);
  if (enable == true)
  {
    gpio_set_level(PIN_PHY_POWER, 1);
    ESP_LOGI(TAG, "phy_device_power_enable(TRUE)");
  }
  else
  {
    gpio_set_level(PIN_PHY_POWER, 0);
    ESP_LOGI(TAG, "power_enable(FALSE)");
  }

  // Allow the power up/down to take effect, min 300us
  vTaskDelay(1);

  if (enable)
  {
    /* Run the PHY-specific power on operations now the PHY has power */
    DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable(true);
  }
}

static void eth_gpio_config_rmii(void)
{
  // RMII data pins are fixed:
  // TXD0 = GPIO19
  // TXD1 = GPIO22
  // TX_EN = GPIO21
  // RXD0 = GPIO25
  // RXD1 = GPIO26
  // CLK == GPIO0
  phy_rmii_configure_data_interface_pins();
  // MDC is GPIO 23, MDIO is GPIO 18
  ESP_LOGI(TAG, "MDC pin:%d", PIN_SMI_MDC);
  ESP_LOGI(TAG, "MDIO pin:%d", PIN_SMI_MDIO);
  phy_rmii_smi_configure_pins(PIN_SMI_MDC, PIN_SMI_MDIO);
}

/**
* @brief event handler for ethernet
*
* @param ctx
* @param event
* @return esp_err_t
*/
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  tcpip_adapter_ip_info_t ip;

  switch (event->event_id)
  {
  case SYSTEM_EVENT_ETH_CONNECTED:
    ESP_LOGI(TAG, "Ethernet Link Up");
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    gotETH_IP = false;
    startWifi();
    ESP_LOGI(TAG, "Ethernet Link Down");
    break;
  case SYSTEM_EVENT_ETH_START:
    ESP_LOGI(TAG, "Ethernet Started");
    tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, "VM208");
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
    ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip));
    ESP_LOGI(TAG, "Ethernet Got IP Addr");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip.ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip.netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip.gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
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
    ESP_LOGI(TAG, "got ip:%s",
             ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
    xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    xEventGroupSetBits(s_wifi_event_group, GOTIP_BIT);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    esp_wifi_connect();
    xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT | GOTIP_BIT);
    gotSTA_IP = false;
    break;
  default:
    break;
  }
  return ESP_OK;
}

static void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
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
      esp_wifi_stop();
    }
    const char* ntpServer = "pool.ntp.org";
    long gmtOffset_sec = 0;
    int daylightOffset_sec= 3600;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void  startWifi()
{

  if (config.getSSID().equals("") || config.getWifiPassword().equals(""))
  {
    ESP_LOGI(TAG, "INVALID WIFI CREDENTIALS");
  }
  else
  {
    if (!config.getWIFI_DHCPEnable())
    {
      ESP_LOGI(TAG,"WIFI DHCP DISABLED");
      Serial.println(config.getWIFI_IPAddress());
      Serial.println(config.getWIFI_Gateway());
      Serial.println(config.getWIFI_SubnetMask());
      tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
      //static ip
      tcpip_adapter_ip_info_t info;
      char ip[15];
      char gw[15];
      char subnet[15];
      config.getWIFI_IPAddress().toCharArray(ip, 15);
      config.getWIFI_Gateway().toCharArray(gw, 15);
      config.getWIFI_SubnetMask().toCharArray(subnet, 15);
      info.ip.addr = ipaddr_addr(ip);
      info.gw.addr = ipaddr_addr(gw);
      info.netmask.addr = ipaddr_addr(subnet);
      tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &info);
      tcpip_adapter_dns_info_t info_dns;
      char dns[15];
      config.getWIFI_PrimaryDNS().toCharArray(dns, 15);
      info_dns.ip.type = IPADDR_TYPE_V4;
      info_dns.ip.u_addr.ip4.addr = ipaddr_addr(dns);
      tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_MAIN, &info_dns);
      config.getWIFI_SecondaryDNS().toCharArray(dns, 15);
      info_dns.ip.type = IPADDR_TYPE_V4;
      info_dns.ip.u_addr.ip4.addr = ipaddr_addr(dns);
      tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_BACKUP, &info_dns);
    }
    char ssid_arr[32];
    char pw[64];
    config.getSSID().toCharArray(ssid_arr, 32);
    config.getWifiPassword().toCharArray(pw, 64);
    ESP_LOGI(TAG, "SSID: %s", ssid_arr);
    ESP_LOGI(TAG, "PW: %s", pw);
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));
    wifi_config_t cfg;
    esp_wifi_get_config(ESP_IF_WIFI_STA, &cfg);
    strncpy((char *)cfg.sta.password, pw, sizeof(cfg.sta.password));
    strncpy((char *)cfg.sta.ssid, ssid_arr, sizeof(cfg.sta.ssid));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
    //Load Config and connect to Wifi
  }
}
static void applyEthNetworkSettings()
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
  else{
    Serial.println("ETH DHCP ENABLED");
    tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_ETH);
  }
}

void startEth()
{
  esp_err_t ret;
  applyEthNetworkSettings();
  eth_config_t eth_config = DEFAULT_ETHERNET_PHY_CONFIG;
  /* Set the PHY address in the example configuration */
  eth_config.phy_addr = PHY0;
  eth_config.gpio_config = eth_gpio_config_rmii;
  eth_config.tcpip_input = tcpip_adapter_eth_input;
  eth_config.clock_mode = ETH_CLOCK_GPIO0_IN;
  eth_config.phy_power_enable = phy_device_power_enable_via_gpio;
  ret = esp_eth_init(&eth_config);

  if (ret == ESP_OK)
  {
    esp_eth_enable();
  }
}

void setup()
{
  g_Mutex = xSemaphoreCreateMutex();
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(10);
  Serial.printf("START\n");

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_LOGI(TAG, "APP MAIN ENTRY");
  Init_IO();

  esp_err_t ret = ESP_OK;
  tcpip_adapter_init();
  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

  SPIFFS.begin();

  Input *inputs;
  inputs = getCurrentInputs();
  readInputs(inputs);
  //check if button 1 and 4 is pressed
  //start AP for WiFi Config
  ESP_LOGI(TAG, "Check Buttons");
  if ((inputs[0].read() == false) && (inputs[3].read() == false))
  {
    ESP_LOGI(TAG, "Start AP");
    WiFi.softAP("VM208_AP", "VellemanForMakers");
    WiFi.enableSTA(false);
  }
  else
  {
    xTaskCreate(IO_task, "IO_task", 2048, NULL, (tskIDLE_PRIORITY + 2), NULL);
    config.load();
    startEth();
    startWifi();
    

    xTaskCreate(got_ip_task, "got_ip_task", 2048, NULL, (tskIDLE_PRIORITY + 2), NULL);
    xEventGroupWaitBits(s_wifi_event_group, GOTIP_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

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
        packet.printf("Aloha, My Name is:%s", config.getBoardName());
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
  startServer();
}

void loop()
{
  ArduinoOTA.handle();
  //delay(5000);
  //printLocalTime();
}


/*
void applyWifiNetworkSettings()
{

}*/
