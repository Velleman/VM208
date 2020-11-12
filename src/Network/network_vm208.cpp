#include "network_VM208.hpp"
#include "time_VM208.hpp"
#include "config_vm208.hpp"
#include "global.hpp"

void applyEthNetworkSettings()
{
  if (!config.getETH_DHCPEnable())
  {
    IPAddress local_IP;
    local_IP.fromString(config.getETH_IPAddress());
    IPAddress gateway;
    gateway.fromString(config.getETH_Gateway());
    IPAddress subnet;
    subnet.fromString(config.getETH_SubnetMask());
    IPAddress primaryDNS;
    primaryDNS.fromString(config.getETH_PrimaryDNS());
    IPAddress secondaryDNS;
    secondaryDNS.fromString(config.getETH_SecondaryDNS());
    ETH.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  }
  else
  {
    IPAddress local;
    local.fromString("0.0.0.0");
    IPAddress gateway;
    gateway.fromString("0.0.0.0");
    IPAddress subnet;
    subnet.fromString("0.0.0.0");
    ETH.config(local, gateway, subnet);
  }
}

void applyWifiNetworkSettings()
{
  if (!config.getWIFI_DHCPEnable())
  {
    IPAddress local_IP;
    local_IP.fromString(config.getWIFI_IPAddress());
    IPAddress gateway;
    gateway.fromString(config.getWIFI_Gateway());
    IPAddress subnet;
    subnet.fromString(config.getWIFI_SubnetMask());
    IPAddress primaryDNS;
    primaryDNS.fromString(config.getWIFI_PrimaryDNS());
    IPAddress secondaryDNS;
    secondaryDNS.fromString(config.getWIFI_SecondaryDNS());
    WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  }
  else
  {
    IPAddress local;
    local.fromString("0.0.0.0");
    IPAddress gateway;
    gateway.fromString("0.0.0.0");
    IPAddress subnet;
    subnet.fromString("0.0.0.0");
    WiFi.config(local, gateway, subnet);
  }
}

bool startEth()
{
  pinMode(GPIO_NUM_2, OUTPUT);
  digitalWrite(GPIO_NUM_2, HIGH);
  ETH.begin(0, PIN_PHY_POWER, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
  if (!config.getETH_DHCPEnable())
  {
    IPAddress local_IP;
    local_IP.fromString(config.getETH_IPAddress());
    IPAddress gateway;
    gateway.fromString(config.getETH_Gateway());
    IPAddress subnet;
    subnet.fromString(config.getETH_SubnetMask());
    IPAddress primaryDNS;
    primaryDNS.fromString(config.getETH_PrimaryDNS());
    IPAddress secondaryDNS;
    secondaryDNS.fromString(config.getETH_SecondaryDNS());
    ETH.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  }

  return ETH.linkUp();
  //return false;
}

void startWifi()
{

  if (config.getSSID().equals("") && config.getWifiPassword().equals(""))
  {
    ESP_LOGI(TAG, "INVALID WIFI CREDENTIALS");
  }
  else
  {
    if (!config.getWIFI_DHCPEnable())
    {
      IPAddress local_IP;
      local_IP.fromString(config.getWIFI_IPAddress());
      IPAddress gateway;
      gateway.fromString(config.getWIFI_Gateway());
      IPAddress subnet;
      subnet.fromString(config.getWIFI_SubnetMask());
      IPAddress primaryDNS;
      primaryDNS.fromString(config.getWIFI_PrimaryDNS());
      IPAddress secondaryDNS;
      secondaryDNS.fromString(config.getWIFI_SecondaryDNS());

      if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
      {
        Serial.println("STA Failed to configure");
      }
    }
    char ssid[100];
    char pw[100];
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
    config.getSSID().toCharArray(ssid, 100);
    config.getWifiPassword().toCharArray(pw, 100);
    Serial.println("Connecting to:");
    Serial.println(config.getSSID());
    Serial.println(config.getWifiPassword());
    if (config.getWifiPassword() != "")
        WiFi.begin(ssid, pw);
    else
    {
      WiFi.begin(ssid);
    }
    
    WiFi.setSleep(false);
  }
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

void got_ip_task(void *pvParameter)
{
  //tcpip_adapter_ip_info_t ip;
  //esp_err_t err;
  while (true)
  {
    xEventGroupWaitBits(s_wifi_event_group, GOTIP_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    if (gotETH_IP)
    {
      //esp_wifi_stop();
      WiFi.disconnect();
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
    //WiFi.mode(WIFI_MODE_NULL);
    ESP_LOGI(TAG, "Ethernet Link Up");
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    gotETH_IP = false;
    ESP_LOGI(TAG, "Ethernet Link Down");
    //startWifi();
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
    WiFi.setHostname("VM208");
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
    if (gotETH_IP == false)
    {
      Serial.println("ETH GOT IP");
      WiFi.reconnect();
    }
    break;
  default:
    break;
  }
  //return ESP_OK;
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