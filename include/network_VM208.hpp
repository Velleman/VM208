#ifndef NETWORK_VM208_HPP
#define NETWORK_VM208_HPP
#include <Arduino.h>
#include <WiFi.h>
#include "eth_phy/phy_lan8720.h"
#define DEFAULT_ETHERNET_PHY_CONFIG phy_lan8720_default_ethernet_config
#include <ETH.h>


#define PIN_PHY_POWER GPIO_NUM_5
#define PIN_SMI_MDC 23
#define PIN_SMI_MDIO 18
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const int GOTIP_BIT = BIT2;

void applyWifiNetworkSettings();
void applyEthNetworkSettings();
bool startEth();
void startWifi();
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void got_ip_task(void *pvParameter);
void WiFiEvent(WiFiEvent_t event);
#endif