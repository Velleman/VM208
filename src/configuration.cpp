#include "configuration.h"

#include "SPIFFS.h"
#include "ArduinoJson.h"

static const char * TAG = "CONFIGURATION";

Configuration::Configuration(){
    _boardName = "VM208";
}

Configuration::~Configuration(){

}

void Configuration::setSSID(String ssid)
{
    if(ssid.length() < MAX_SSID_LENGTH)
    {
        _ssid = ssid;
    }
}

String Configuration::getSSID() const{
    return _ssid;
}

void Configuration::setWifiPassword(String password){
    if(password.length() < MAX_WIFI_PASSWORD_LENGTH)
    {
        _wifi_pw = password;
    }

}

String Configuration::getWifiPassword() const{
    return _wifi_pw;
}

void Configuration::setBoardName(String boardName){
    if(boardName.length() < MAX_BOARDNAME_LENGTH)
    {
        _boardName = boardName;
    }
}

String Configuration::getBoardName() const{
    return _boardName;
}

void Configuration::load(){
    StaticJsonBuffer<1024> jsonBuffer;

    if(SPIFFS.exists(configPath))
    {
        ESP_LOGI(TAG,"file exist");
        File file = loadFile(configPath);
        JsonObject &root = jsonBuffer.parseObject(file);
        _ssid = root[SSID_KEY].as<String>();
        _wifi_pw = root[WIFI_PW_KEY].as<String>();
        _boardName = root[BOARDNAME_KEY].as<String>();
        Serial.println(_ssid);
        Serial.println(_wifi_pw);
        Serial.println(_boardName);
    }
    else{
        ESP_LOGI(TAG,"file doesnt exist");
    }
}

void Configuration::save()
{
   writeFile(configPath);
}

void Configuration::writeFile(const char * path){
    if(SPIFFS.exists(configPath))
    {
        ESP_LOGI(TAG,"remove file");
        SPIFFS.remove(path);
    }
    JsonObject& root = jsonBuffer.createObject();
    root[SSID_KEY] = _ssid;
    root[WIFI_PW_KEY] = _wifi_pw;
    root[BOARDNAME_KEY] = _boardName;
    File file = SPIFFS.open(path,"w");
    root.printTo(file);
    file.close();
}

File Configuration::loadFile(const char * path)
{
    return SPIFFS.open(path);
}