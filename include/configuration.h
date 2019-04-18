#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H
#include "stdint.h"
#include "Arduino.h"
#include "FS.h"
#include "ArduinoJson.h"
class Configuration
{
  private:
    #define MAX_SSID_LENGTH 32
    #define MAX_WIFI_PASSWORD_LENGTH 64
    #define MAX_BOARDNAME_LENGTH  32
    #define SSID_KEY "SSID"
    #define WIFI_PW_KEY "WiFi_PW"
    #define BOARDNAME_KEY "BOARDNAME"
    String _ssid;
    String _wifi_pw;
    String _boardName;
    const char* configPath = "/config.json";
    void writeFile(const char * path);
    File loadFile(const char * path);
    StaticJsonBuffer<1024> jsonBuffer;
  public:
    Configuration();
    
    //getter setter SSID
    String getSSID() const;
    void setSSID(String ssid);

    //getter setter WifiPassword
    String getWifiPassword() const;
    void setWifiPassword(String password);

    //getter setter BoarName
    String getBoardName() const;
    void setBoardName(String boardname);

    void load();
    void save();

    ~Configuration();

};

#endif