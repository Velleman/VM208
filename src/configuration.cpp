#include "configuration.hpp"

#include "SPIFFS.h"
#include "ArduinoJson.h"

static const char *TAG = "CONFIGURATION";

Configuration::Configuration()
{
    _boardName = "VM208";
}

Configuration::~Configuration()
{
}

void Configuration::setSSID(String ssid)
{
    if (ssid.length() < MAX_SSID_LENGTH)
    {
        _ssid = ssid;
    }
}

String Configuration::getSSID() const
{
    return _ssid;
}

void Configuration::setWifiPassword(String password)
{
    if (password.length() < MAX_WIFI_PASSWORD_LENGTH)
    {
        _wifi_pw = password;
    }
}

String Configuration::getWifiPassword() const
{
    return _wifi_pw;
}

void Configuration::setBoardName(String boardName)
{
    if (boardName.length() < MAX_BOARDNAME_LENGTH)
    {
        _boardName = boardName;
    }
}

String Configuration::getBoardName() const
{
    return _boardName;
}

void Configuration::setUserName(String username)
{
    if (username.length() < MAX_USERNAME_LENGTH)
    {
        _username = username;
    }
}

String Configuration::getUserName() const
{
    return _username;
}

void Configuration::setUserPw(String pw)
{
    if (pw.length() < MAX_USERNAME_LENGTH)
    {
        _userPW = pw;
    }
}

String Configuration::getUserPw() const
{
    return _userPW;
}

void Configuration::load()
{

    if (SPIFFS.exists(configPath))
    {
        ESP_LOGI(TAG, "file exist");
        File file = loadFile(configPath);
        JsonObject &root = jsonBuffer.parseObject(file);
        _ssid = root[SSID_KEY].as<String>();
        _wifi_pw = root[WIFI_PW_KEY].as<String>();
        _boardName = root[BOARDNAME_KEY].as<String>();
        _username = root[USERNAME_KEY].as<String>();
        _userPW = root[USERPW_KEY].as<String>();
        _WIFI_DHCPEnable = root[WIFI_DHCPEN_KEY].as<bool>();
        _ETH_DHCPEnable = root[ETH_DHCPEN_KEY].as<bool>();
    }
    else
    {
        ESP_LOGI(TAG, "file doesnt exist");
    }
}

void Configuration::save()
{
    writeFile(configPath);
}

void Configuration::writeFile(const char *path)
{
    if (SPIFFS.exists(configPath))
    {
        ESP_LOGI(TAG, "remove file");
        SPIFFS.remove(path);
    }
    JsonObject &root = jsonBuffer.createObject();
    root[SSID_KEY] = _ssid;
    root[WIFI_PW_KEY] = _wifi_pw;
    root[BOARDNAME_KEY] = _boardName;
    root[USERNAME_KEY] = _username;
    root[USERPW_KEY] = _userPW;
    root[ETH_DHCPEN_KEY] = _ETH_DHCPEnable;
    root[WIFI_DHCPEN_KEY] = _WIFI_DHCPEnable;
    File file = SPIFFS.open(path, "w");
    root.printTo(file);
    file.close();
}

File Configuration::loadFile(const char *path)
{
    return SPIFFS.open(path);
}

bool Configuration::getETH_DHCPEnable() const
{
    return _ETH_DHCPEnable;
}

void Configuration::setETH_DHCPEnable(bool dhcpenable)
{
    _ETH_DHCPEnable = dhcpenable;
}

String Configuration::getETH_IPAddress() const
{
    return _ETH_IPAddress;
}

void Configuration::setETH_IPAddress(String ip)
{
    _ETH_IPAddress = ip;
}

String Configuration::getETH_Gateway() const
{
    return _ETH_Gateway;
}

void Configuration::setETH_Gateway(String gateway)
{
    _ETH_Gateway = gateway;
}

String Configuration::getETH_SubnetMask() const
{
    return _ETH_SubnetMask;
}

void Configuration::setETH_SubnetMask(String subnetmask)
{
    _ETH_SubnetMask = subnetmask;
}

String Configuration::getETH_PrimaryDNS() const
{
    return _ETH_PrimaryDNS;
}

void Configuration::setETH_PrimaryDNS(String primaryDNS)
{
    _ETH_PrimaryDNS = primaryDNS;
}

String Configuration::getETH_SecondaryDNS() const
{
    return _ETH_SecondaryDNS;
}

void Configuration::setETH_SecondaryDNS(String secondaryDNS)
{
    _ETH_SecondaryDNS = secondaryDNS;
}

bool Configuration::getWIFI_DHCPEnable() const
{
    return _WIFI_DHCPEnable;
}

void Configuration::setWIFI_DHCPEnable(bool dhcpenable)
{
    _WIFI_DHCPEnable = dhcpenable;
}

String Configuration::getWIFI_IPAddress() const
{
    return _WIFI_IPAddress;
}

void Configuration::setWIFI_IPAddress(String ip)
{
    _WIFI_IPAddress = ip;
}

String Configuration::getWIFI_Gateway() const
{
    return _WIFI_Gateway;
}

void Configuration::setWIFI_Gateway(String gateway)
{
    _WIFI_Gateway = gateway;
}

String Configuration::getWIFI_SubnetMask() const
{
    return _WIFI_SubnetMask;
}

void Configuration::setWIFI_SubnetMask(String subnetmask)
{
    _WIFI_SubnetMask = subnetmask;
}

String Configuration::getWIFI_PrimaryDNS() const
{
    return _WIFI_PrimaryDNS;
}

void Configuration::setWIFI_PrimaryDNS(String primaryDNS)
{
    _WIFI_PrimaryDNS = primaryDNS;
}

String Configuration::getWIFI_SecondaryDNS() const
{
    return _WIFI_SecondaryDNS;
}

void Configuration::setWIFI_SecondaryDNS(String secondaryDNS)
{
    _WIFI_SecondaryDNS = secondaryDNS;
}

const char* Configuration::SSID_KEY = "SSID";
const char* Configuration::WIFI_PW_KEY = "WiFi_PW";
const char* Configuration::BOARDNAME_KEY = "BOARDNAME";
const char* Configuration::USERNAME_KEY = "USERNAME";
const char* Configuration::USERPW_KEY = "USERPW";
const char* Configuration::ETH_DHCPEN_KEY =         "ETH_DHCPEN";
const char* Configuration::ETH_IPADDR_KEY =         "ETH_IPADDR";
const char* Configuration::ETH_GATEWAY_KEY =        "ETH_GATEWAY";
const char* Configuration::ETH_SUBNETMASK_KEY =     "ETH_SUBNET";
const char* Configuration::ETH_PRIMARYDNS_KEY =     "ETH_PRIMARYDNS";
const char* Configuration::ETH_SECONDARYDNS_KEY =   "ETH_SECONDARYDNS";
const char* Configuration::WIFI_DHCPEN_KEY =        "WIFI_DHCPEN";
const char* Configuration::WIFI_IPADDR_KEY =        "WIFI_IPADDR";
const char* Configuration::WIFI_GATEWAY_KEY =       "WIFI_GATEWAY";
const char* Configuration::WIFI_SUBNETMASK_KEY =    "WIFI_SUBNET";
const char* Configuration::WIFI_PRIMARYDNS_KEY =    "WIFI_PRIMARYDNS";
const char* Configuration::WIFI_SECONDARYDNS_KEY =  "WIFI_SECONDARYDNS";
