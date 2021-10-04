#ifndef _CONFIG_VM208_HPP
#define _CONFIG_VM208_HPP

#include "stdint.h"
#include "Arduino.h"
#include "FS.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"
//#include "relay.hpp"
//#include "mosfet.hpp"
//#include "VM208TimerChannel.hpp"
#include "ChannelShedule.hpp"
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
  String _version = "1.1.0";
  long _timezoneSeconds;
  int _DSTseconds;
  int _timezone_id;
  const char *configPath = "/config.json";
  const char *alarmPath = "/alarms/";
  const char *emailPath = "/email.json";
  const char *namesPath = "/names.json";
  void writeConfig();
  void writeAlarms();
  
  void writeEmailSettings();
  File loadFile(const char *path);
  //DynamicJsonBuffer jsonBuffer;
  String m_channelNames[14];
  bool _firstTime;
  //mail
  String _email_server;
  String _email_port;
  String _email_user;
  String _email_pw;
  String _email_recipient;
  String _email_subject;

  String _name_input;
  String _mosfet1_name;
  String _mosfet2_name;

  bool _notif_boot;
  bool _notif_input_change;
  bool _notif_ext_connected;
  bool _notif_manual_input;
  String _names[268];
  ChannelShedule _cs[268];
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
  static const char *TIMEZONEID_KEY;
  static const char *TIMEZONE_KEY;
  static const char *DST_KEY;
  static const char *ALARM_WEEKDAY_KEY;
  static const char *ALARM_HOUR_KEY;
  static const char *ALARM_MINUTE_KEY;
  static const char *ALARM_STATE_KEY;
  static const char *ALARM_ENABLED_KEY;
  static const char *CHANNEL_NAME_KEY;
  static const char *FIRST_TIME_KEY;
  static const char *EMAIL_SERRVER_KEY;
  static const char *EMAIL_PORT_KEY;
  static const char *EMAIL_USER_KEY;
  static const char *EMAIL_PW_KEY;
  static const char *EMAIL_RECEIVER_KEY;
  static const char *EMAIL_TITLE_KEY;
  static const char *NAME_INPUT_KEY;
  static const char *NAME_MOSFET1_KEY;
  static const char *NAME_MOSFET2_KEY;
  static const char *NOTIF_BOOT_KEY;
  static const char *NOTIF_INPUT_CHANGE_KEY;
  static const char *NOTIF_EXT_CONNECT_KEY;
  static const char *NOTIF_MANUAL_INPUT_KEY;

  void writeAlarm(uint16_t id);

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

  //Channel createChannel(uint8_t id, Relay *r, Led *l);
  //Channel createMosfetChannel(uint8_t id, Mosfet *r);

  bool getFirstTime();
  void setFirstTime(bool first_time);

  String getEmailServer();
  void setEmailServer(String server);

  String getEmailPort();
  void setEmailPort(String port);

  String getEmailUser();
  void setEmailUser(String user);

  String getEmailPW();
  void setEmailPW(String pw);

  String getEmailRecipient();
  void setEmailRecipient(String recipient);

  String getEmailSubject();
  void setEmailSubject(String subject);

  String getInputName();
  void setInputName(String name);

  String getMosfet1Name();
  void setMosfet1Name(String name);

  String getMosfet2Name();
  void setMosfet2Name(String name);

  bool getNotificationBoot();
  void setNotificationBoot(bool enable);

  bool getNotificationInputChange();
  void setNotificationInputChange(bool enable);

  bool getNotification_ext_connected();
  void setNotification_ext_connected(bool enable);

  bool getNotification_manual_input();
  void setNotification_manual_input(bool enable);

  void setTimeZoneID(int timezoneID);
  int getTimeZoneID();
  void setDST(int seconds);
  int getDST();
  void load();
  void loadAlarms();
  void save();
  void saveAlarms();
  void saveEmailSettings();
  String getNameFromChannel(uint16_t index);
  void saveNames();
  void setName(uint16_t channelID,String name);

  void setShedule(uint16_t channel,uint8_t dayOfWeek,uint8_t hour,uint8_t min,bool onOff,bool enbale);
  ChannelShedule* getShedule(uint16_t index);
  ~Configuration();
};

#endif