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

#include "IO.h"

#include "tcpip_adapter.h"
#include "soc/emac_ex_reg.h"
#include "driver/periph_ctrl.h"
#include "WifiConfig.h"

#include "AsyncUDP.h"

#include "eth_phy/phy_lan8720.h"
#define DEFAULT_ETHERNET_PHY_CONFIG phy_lan8720_default_ethernet_config
#include <SPIFFS.h>
#include "configuration.h"

static const char *TAG = "VM208_MAIN";

extern bool gotETH_IP = false;
extern bool gotSTA_IP = false;

#define PIN_PHY_POWER GPIO_NUM_5
#define PIN_SMI_MDC 23
#define PIN_SMI_MDIO 18

extern Configuration config;

String processor(const String &var)
{
  /*if(var == "NAME")
    return F("VM208");
  if(var == "MAC")
    return F("MACDonals");
  if(var == "UPTIME")
    return F("tis al lang");
  if(var == "FIRMWAREVERSION")
    return F("V0.0.1");*/
  return String("test");
}

const char *ssid = "Eminent_RnD";
const char *password = "2017wifi";
const char *hostName = "esp-async";

AsyncUDP udp;

Configuration config;

void startWifi();

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
    //ESP_LOGI(TAG, "SMARTCONFIG_STOP");
    //esp_smartconfig_stop();
    //ESP_LOGI(TAG, "LOOPTY SWOOPTY");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void startWifi()
{
  
  if (config.getSSID().equals("") || config.getWifiPassword().equals(""))
  {
    ESP_LOGI(TAG, "INVALID WIFI CREDENTIALS");
    char ssid[32];
    char pw[64];
    config.getSSID().toCharArray(ssid, 32);
    config.getWifiPassword().toCharArray(pw, 64);
    ESP_LOGI(TAG, "SSID: %s", ssid);
    ESP_LOGI(TAG, "PW: %s", pw); 
  }
  else
  {
    char ssid[32];
    char pw[64];
    config.getSSID().toCharArray(ssid, 32);
    config.getWifiPassword().toCharArray(pw, 64);
    ESP_LOGI(TAG, "SSID: %s", ssid);
    ESP_LOGI(TAG, "PW: %s", pw);
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));
    wifi_config_t cfg;
    esp_wifi_get_config(ESP_IF_WIFI_STA,&cfg);
    strncpy((char *)cfg.ap.password, pw, sizeof(cfg.ap.password));
    strncpy((char *)cfg.ap.ssid, ssid, sizeof(cfg.ap.ssid));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
    //Load Config and connect to Wifi
  }
}

void setup()
{
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

  input_t *inputs;
  inputs = getCurrentInputs();
  readInputs(inputs);
  //check if button 1 and 4 is pressed
  //start AP for WiFi Config
  ESP_LOGI(TAG, "Check Buttons");
  if ((inputs[0].state == INPUT_ON) && (inputs[3].state == INPUT_ON))
  {
    ESP_LOGI(TAG, "Start AP");
    WiFi.softAP("VM208_AP", "VellemanForMakers");
    WiFi.enableSTA(false);
  }
  else
  {
    xTaskCreate(IO_task, "IO_task", 2048, NULL, (tskIDLE_PRIORITY + 2), NULL);
    config.load();
    startWifi();

    eth_config_t config = DEFAULT_ETHERNET_PHY_CONFIG;
    /* Set the PHY address in the example configuration */
    config.phy_addr = PHY0;
    config.gpio_config = eth_gpio_config_rmii;
    config.tcpip_input = tcpip_adapter_eth_input;
    config.clock_mode = ETH_CLOCK_GPIO0_IN;
    config.phy_power_enable = phy_device_power_enable_via_gpio;
    ret = esp_eth_init(&config);

    if (ret == ESP_OK)
    {
      esp_eth_enable();
    }

    xTaskCreate(got_ip_task, "got_ip_task", 2048, NULL, (tskIDLE_PRIORITY + 2), NULL);
    xEventGroupWaitBits(s_wifi_event_group, GOTIP_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

    //Serial.print ( "IP address: " );
    //Serial.println ( WiFi.localIP() );

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
        packet.printf("Got %u bytes of data", packet.length());
      });
    }
    if (!MDNS.begin("VM208"))
    {
      Serial.println("Error setting up MDNS responder!");
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
}
