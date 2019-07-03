#ifndef _CONFIG_VM208_HPP
#define _CONFIG_VM208_HPP

#include "stdint.h"
#include "Arduino.h"
#include "FS.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"
#include "channel.hpp"
class Configuration
{
private:
#define MAX_SSID_LENGTH 32
#define MAX_WIFI_PASSWORD_LENGTH 64
#define MAX_BOARDNAME_LENGTH 32
#define MAX_USERNAME_LENGTH 32

  String _ssid;
  String _wifi_pw;
  String _boardName;
  String _username;
  String _userPW;
  bool _ETH_DHCPEnable;
  String _ETH_IPAddress;
  String _ETH_Gateway;
  String _ETH_SubnetMask;
  String _ETH_PrimaryDNS;
  String _ETH_SecondaryDNS;
  bool _WIFI_DHCPEnable;
  String _WIFI_IPAddress;
  String _WIFI_Gateway;
  String _WIFI_SubnetMask;
  String _WIFI_PrimaryDNS;
  String _WIFI_SecondaryDNS;
  String _version = "0.1.1";
  long _timezoneSeconds;
  int _DSTseconds;
  const char *configPath = "/config.json";
  void writeFile(const char *path);
  File loadFile(const char *path);
  DynamicJsonBuffer jsonBuffer;
  String m_channelNames[14];
  bool _firstTime;
public:
  Configuration();
  static const char *SSID_KEY;
  static const char *WIFI_PW_KEY;
  static const char *BOARDNAME_KEY;
  static const char *USERNAME_KEY;
  static const char *USERPW_KEY;
  static const char *ETH_DHCPEN_KEY;
  static const char *ETH_IPADDR_KEY;
  static const char *ETH_GATEWAY_KEY;
  static const char *ETH_SUBNETMASK_KEY;
  static const char *ETH_PRIMARYDNS_KEY;
  static const char *ETH_SECONDARYDNS_KEY;
  static const char *WIFI_DHCPEN_KEY;
  static const char *WIFI_IPADDR_KEY;
  static const char *WIFI_GATEWAY_KEY;
  static const char *WIFI_SUBNETMASK_KEY;
  static const char *WIFI_PRIMARYDNS_KEY;
  static const char *WIFI_SECONDARYDNS_KEY;
  static const char *VERSION_KEY;
  static const char *TIMEZONE_KEY;
  static const char *DST_KEY;
  static const char *ALARM_WEEKDAY_KEY;
  static const char *ALARM_HOUR_KEY;
  static const char *ALARM_MINUTE_KEY;
  static const char *ALARM_STATE_KEY;
  static const char *ALARM_ENABLED_KEY;
  static const char *CHANNEL_NAME_KEY;
  static const char *FIRST_TIME_KEY;
  //getter setter SSID
  String getVersion() const;

  //getter setter SSID
  String getSSID() const;
  void setSSID(String ssid);

  //getter setter WifiPassword
  String getWifiPassword() const;
  void setWifiPassword(String password);

  //getter setter BoarName
  String getBoardName() const;
  void setBoardName(String boardname);

  //getter setter UserName
  String getUserName() const;
  void setUserName(String username);

  //getter setter userpw
  String getUserPw() const;
  void setUserPw(String userpw);

  //getter setter userpw
  bool getETH_DHCPEnable() const;
  void setETH_DHCPEnable(bool enable);

  //getter setter userpw
  String getETH_IPAddress() const;
  void setETH_IPAddress(String enable);

  //getter setter userpw
  String getETH_Gateway() const;
  void setETH_Gateway(String enable);

  //getter setter userpw
  String getETH_SubnetMask() const;
  void setETH_SubnetMask(String enable);

  //getter setter userpw
  String getETH_PrimaryDNS() const;
  void setETH_PrimaryDNS(String enable);

  //getter setter userpw
  String getETH_SecondaryDNS() const;
  void setETH_SecondaryDNS(String enable);

  //getter setter userpw
  bool getWIFI_DHCPEnable() const;
  void setWIFI_DHCPEnable(bool enable);

  //getter setter userpw
  String getWIFI_IPAddress() const;
  void setWIFI_IPAddress(String enable);

  //getter setter userpw
  String getWIFI_Gateway() const;
  void setWIFI_Gateway(String enable);

  //getter setter userpw
  String getWIFI_SubnetMask() const;
  void setWIFI_SubnetMask(String enable);

  //getter setter userpw
  String getWIFI_PrimaryDNS() const;
  void setWIFI_PrimaryDNS(String enable);

  //getter setter userpw
  String getWIFI_SecondaryDNS() const;
  void setWIFI_SecondaryDNS(String enable);

  void setTimezone(long seconds);
  long getTimezone();

  void getAlarm(uint8_t channel, uint8_t index);

  String getChannelNameById(uint8_t id);

  Channel createChannel(uint8_t id, Relay *r, Led *l);

  bool getFirstTime();
  void setFirstTime(bool first_time);

  void setDST(int seconds);
  int getDST();
  void load();
  void save();

  ~Configuration();
};

#endif