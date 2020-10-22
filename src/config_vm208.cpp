#include "config_vm208.hpp"

#include "SPIFFS.h"
#include "ArduinoJson.h"
#include "alarm.hpp"
#include "IO.hpp"
static const char *TAG = "CONFIGURATION";

Configuration::Configuration()
{
    _boardName = "VM208";
    _DSTseconds = 0;
    _timezoneSeconds = 0;
    _ETH_DHCPEnable = true;
    _WIFI_DHCPEnable = true;
    _firstTime = true;
}

Configuration::~Configuration()
{
}

String Configuration::getVersion() const
{
    return _version;
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

void Configuration::loadAlarms()
{
    if (true)
    {

        ESP_LOGI(TAG, "file exist");

        //StaticJsonBuffer<300000> jsonBuffer;

        for (uint16_t i = 0; i < 268; i++)
        {
            String path = alarmPath;
            path += i;
            path += ".json";
            File file = loadFile(path.c_str());
            DynamicJsonBuffer jsonBuffer;
            JsonObject &root = jsonBuffer.parseObject(file);
            JsonArray &alarms = root["alarms"];

            auto channel = mm.getChannel(i + 1);
            if (channel != nullptr)
            {
                Serial.printf("Channel Shedule index: %d\r\n",i);
                _cs[i].setChannel(channel);
                _cs[i].setChannelID(i+1);
                for (uint8_t j = 0; j < 14; j++)
                {
                    JsonObject &Channels_0_alarms_i = alarms[j];
                    uint dow = Channels_0_alarms_i[ALARM_WEEKDAY_KEY];
                    uint hour = Channels_0_alarms_i[ALARM_HOUR_KEY];
                    uint minute = Channels_0_alarms_i[ALARM_MINUTE_KEY];
                    bool enabled = Channels_0_alarms_i[ALARM_ENABLED_KEY];
                    bool onOff = (j < 7);
                    _cs[i].setShedule(dow, hour, minute, onOff, enabled);
                }
                mm.getModuleFromChannelID(i+1)->Disactivate();
            }
            file.close();
            jsonBuffer.clear();
        }
    }
}

void Configuration::load()
{
    if (SPIFFS.exists(configPath))
    {
        ESP_LOGI(TAG, "file exist");
        File file = loadFile(configPath);

        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.parseObject(file);
        if (root.containsKey(SSID_KEY))
        {
            _ssid = root[SSID_KEY].as<String>();
            _wifi_pw = root[WIFI_PW_KEY].as<String>();
            _boardName = root[BOARDNAME_KEY].as<String>();
            _username = root[USERNAME_KEY].as<String>();
            _userPW = root[USERPW_KEY].as<String>();
            _WIFI_DHCPEnable = root[WIFI_DHCPEN_KEY].as<bool>();
            _WIFI_IPAddress = root[WIFI_IPADDR_KEY].as<String>();
            _WIFI_Gateway = root[WIFI_GATEWAY_KEY].as<String>();
            _WIFI_SubnetMask = root[WIFI_SUBNETMASK_KEY].as<String>();
            _WIFI_PrimaryDNS = root[WIFI_PRIMARYDNS_KEY].as<String>();
            _WIFI_SecondaryDNS = root[WIFI_SECONDARYDNS_KEY].as<String>();
            _ETH_DHCPEnable = root[ETH_DHCPEN_KEY].as<bool>();
            _ETH_IPAddress = root[ETH_IPADDR_KEY].as<String>();
            _ETH_Gateway = root[ETH_GATEWAY_KEY].as<String>();
            _ETH_SubnetMask = root[ETH_SUBNETMASK_KEY].as<String>();
            _ETH_PrimaryDNS = root[ETH_PRIMARYDNS_KEY].as<String>();
            _ETH_SecondaryDNS = root[ETH_SECONDARYDNS_KEY].as<String>();
            _timezoneSeconds = root[TIMEZONE_KEY].as<long>();
            _DSTseconds = root[DST_KEY].as<int>();
            _firstTime = root[FIRST_TIME_KEY].as<bool>();
            _name_input = root[NAME_INPUT_KEY].as<String>();
            _mosfet1_name = root[NAME_MOSFET1_KEY].as<String>();
            _mosfet2_name = root[NAME_MOSFET2_KEY].as<String>();
        }
        else
        {
            Serial.println("LOAD CONFIG: FILE IS INVALID");
        }
        file.close();
        jsonBuffer.clear();
    }
    else
    {
        ESP_LOGI(TAG, "file doesnt exist");
    }
    if (SPIFFS.exists(emailPath))
    {
        ESP_LOGI(TAG, "file exist");
        File file = loadFile(emailPath);
        /* while (file.available())
        {
            Serial.write(file.read());
        }*/
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.parseObject(file);
        if (root.containsKey(EMAIL_SERRVER_KEY))
        {
            _email_server = root[EMAIL_SERRVER_KEY].as<String>();
            _email_port = root[EMAIL_PORT_KEY].as<String>();
            _email_user = root[EMAIL_USER_KEY].as<String>();
            _email_pw = root[EMAIL_PW_KEY].as<String>();
            _email_recipient = root[EMAIL_RECEIVER_KEY].as<String>();
            _email_subject = root[EMAIL_TITLE_KEY].as<String>();
            _notif_boot = root[NOTIF_BOOT_KEY].as<bool>();
            _notif_ext_connected = root[NOTIF_EXT_CONNECT_KEY].as<bool>();
            _notif_input_change = root[NOTIF_INPUT_CHANGE_KEY].as<bool>();
            _notif_manual_input = root[NOTIF_MANUAL_INPUT_KEY].as<bool>();
        }
        else
        {
            Serial.println("LOAD EMAIL CONFIG: FILE IS INVALID");
        }
        file.close();
        jsonBuffer.clear();
    }
    else
    {
        ESP_LOGI(TAG, "file doesnt exist");
    }
    if (SPIFFS.exists(namesPath))
    {
        ESP_LOGI(TAG, "file exist");
        File file = loadFile(namesPath);
        /* while (file.available())
        {
            Serial.write(file.read());
        }*/
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.parseObject(file);
        JsonArray &names = root["names"];
        if (root.containsKey("names"))
        {
            for (int i = 0; i < 268; i++)
            {
                _names[i] = names[i].as<String>();
            }
        }
        else
        {
            Serial.println("LOAD NAME CONFIG: FILE IS INVALID");
        }
        file.close();
        jsonBuffer.clear();
    }
    else
    {
        ESP_LOGI(TAG, "file doesnt exist");
    }

    //SPIFFS.end();
}

ChannelShedule *Configuration::getShedule(uint16_t index)
{
    if (index < 268)
        return &_cs[index];
    else
    {
        return nullptr;
    }
}

String Configuration::getNameFromChannel(uint16_t index)
{
    return _names[index];
}

void Configuration::saveNames()
{
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    JsonArray &names = root.createNestedArray("names");

    for (int i = 0; i < 268; i++)
    {
        names.add(_names[i]);
    }
    File file = SPIFFS.open(namesPath, FILE_WRITE);
    if (root.printTo(file) == 0)
    {
        Serial.println("Error writing to file");
    }
    file.close();
    jsonBuffer.clear();
}

void Configuration::setName(uint16_t channelId, String name)
{
    _names[channelId - 1] = name;
}

void Configuration::save()
{
    writeConfig();
}

void Configuration::saveAlarms()
{
    writeAlarms();
}

void Configuration::saveEmailSettings()
{
    writeEmailSettings();
}

void Configuration::writeEmailSettings()
{
    if (SPIFFS.exists(emailPath))
    {
        ESP_LOGI(TAG, "remove file");
        SPIFFS.remove(emailPath);
    }
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[EMAIL_SERRVER_KEY] = _email_server;
    root[EMAIL_PORT_KEY] = _email_port;
    root[EMAIL_USER_KEY] = _email_user;
    root[EMAIL_PW_KEY] = _email_pw;
    root[EMAIL_RECEIVER_KEY] = _email_recipient;
    root[EMAIL_TITLE_KEY] = _email_subject;
    root[NOTIF_BOOT_KEY] = _notif_boot;
    root[NOTIF_EXT_CONNECT_KEY] = _notif_ext_connected;
    root[NOTIF_INPUT_CHANGE_KEY] = _notif_input_change;
    root[NOTIF_MANUAL_INPUT_KEY] = _notif_manual_input;
    File file = SPIFFS.open(emailPath, FILE_WRITE);
    if (root.printTo(file) == 0)
    {
        Serial.println("Error writing to file");
    }
    file.close();
}

/*
{
	"Channels": [{
		"name": "Channel1",
		"alarms": [{
			"alarm1": [{
					"dow": "monday",
					"hour": 12,
					"minute": 10
				},
				{
					"alarm1": [{
						"dow": "monday",
						"hour": 12,
						"minute": 10
					}]
				}
			]
		}]
	}]
}
https://arduinojson.org/v5/assistant/
 */

void Configuration::writeConfig()
{
    if (SPIFFS.exists(configPath))
    {
        Serial.println("remove config file");
        SPIFFS.remove(configPath);
    }
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[SSID_KEY] = _ssid;
    root[WIFI_PW_KEY] = _wifi_pw;
    root[BOARDNAME_KEY] = _boardName;
    root[USERNAME_KEY] = _username;
    root[USERPW_KEY] = _userPW;
    root[ETH_DHCPEN_KEY] = _ETH_DHCPEnable;
    root[ETH_IPADDR_KEY] = _ETH_IPAddress;
    root[ETH_GATEWAY_KEY] = _ETH_Gateway;
    root[ETH_SUBNETMASK_KEY] = _ETH_SubnetMask;
    root[ETH_PRIMARYDNS_KEY] = _ETH_PrimaryDNS;
    root[ETH_SECONDARYDNS_KEY] = _ETH_SecondaryDNS;
    root[WIFI_DHCPEN_KEY] = _WIFI_DHCPEnable;
    root[WIFI_IPADDR_KEY] = _WIFI_IPAddress;
    root[WIFI_GATEWAY_KEY] = _WIFI_Gateway;
    root[WIFI_SUBNETMASK_KEY] = _WIFI_SubnetMask;
    root[WIFI_PRIMARYDNS_KEY] = _WIFI_PrimaryDNS;
    root[WIFI_SECONDARYDNS_KEY] = _WIFI_SecondaryDNS;
    root[TIMEZONE_KEY] = _timezoneSeconds;
    root[DST_KEY] = _DSTseconds;
    root[FIRST_TIME_KEY] = _firstTime;
    root[NAME_INPUT_KEY] = _name_input;
    root[NAME_MOSFET1_KEY] = _mosfet1_name;
    root[NAME_MOSFET2_KEY] = _mosfet2_name;

    File file = SPIFFS.open(configPath, FILE_WRITE);
    root.printTo(file);
    file.close();
}

void Configuration::writeAlarms()
{
    if (SPIFFS.exists(alarmPath))
    {
        ESP_LOGI(TAG, "remove file");
        SPIFFS.remove(alarmPath);
    }
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    JsonArray &Channels = root.createNestedArray("Channels");
    //VM208TimeChannel *c;
    Alarm *a;
    for (int i = 0; i < 12; i++)
    {
        /*c = (VM208TimeChannel *)getRelayChannelById(i + 1);
        JsonObject &Channel = Channels.createNestedObject();
        Channel[CHANNEL_NAME_KEY] = c->getName();

        JsonArray &Channels_alarms = Channel.createNestedArray("alarms");
        for (int j = 0; j < 14; j++)
        {
            a = c->getAlarm(j);
            JsonObject &Channels_alarms_settings = Channels_alarms.createNestedObject();
            Channels_alarms_settings[ALARM_WEEKDAY_KEY] = a->getWeekday();
            Channels_alarms_settings[ALARM_HOUR_KEY] = a->getHour();
            Channels_alarms_settings[ALARM_MINUTE_KEY] = a->getMinute();
            Channels_alarms_settings[ALARM_STATE_KEY] = a->getState();
            Channels_alarms_settings[ALARM_ENABLED_KEY] = a->isEnabled();
        }*/
    }
    //Write json file
    File file = SPIFFS.open(alarmPath, "w");
    root.printTo(file);
    file.close();
}

void Configuration::writeAlarm(uint16_t id)
{
    String path = alarmPath;
    path += id - 1;
    path += ".json";

    if (SPIFFS.exists(path))
    {
        ESP_LOGI(TAG, "remove file");
        SPIFFS.remove(path);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        JsonArray &alarms = root.createNestedArray("alarms");

        for (uint8_t j = 0; j < 14; j++)
        {
            uint8_t dow = j > 6 ? j - 7 : j;
            uint8_t onOff = j < 7;
            Shedule *shedule = _cs[id - 1].getShedule(dow, onOff);
            JsonObject &alarm = alarms.createNestedObject();
            /*alarm["dow"] = shedule->dateTime.tm_wday;
            alarm["hour"] = shedule->dateTime.tm_hour;
            alarm["minute"] = shedule->dateTime.tm_min;*/
            alarm["dow"] = shedule->dow;
            alarm["hour"] = shedule->hour;
            alarm["minute"] = shedule->minute;
            alarm["enabled"] = shedule->enable;
        }
        File file = SPIFFS.open(path, "w");
        root.printTo(file);
        file.close();
        jsonBuffer.clear();
    }
}

/*Channel Configuration::createChannel(uint8_t id, Relay *r, Led *l)
{
    Channel c;
    if (SPIFFS.exists(alarmPath))
    {

        ESP_LOGI(TAG, "file exist");
        File file = loadFile(alarmPath);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.parseObject(file);
        JsonArray &channels = root["Channels"];

        String Channels_i_name = channels[id - 1]["name"]; // "Channel1"
        JsonArray &Channels_i_alarms = channels[id - 1]["alarms"];
        c = Channel(Channels_i_name, r, l, id);
        for (int i = 0; i < 14; i++)
        {
            JsonObject &Channels_0_alarms_i = Channels_i_alarms[i];
            uint dow = Channels_0_alarms_i[ALARM_WEEKDAY_KEY];
            uint hour = Channels_0_alarms_i[ALARM_HOUR_KEY];
            uint minute = Channels_0_alarms_i[ALARM_MINUTE_KEY];
            bool state = Channels_0_alarms_i[ALARM_STATE_KEY];
            bool enabled = Channels_0_alarms_i[ALARM_ENABLED_KEY];
            Alarm a = Alarm(dow, hour, minute, state, enabled);
            c.setAlarm(a, i);
        }
        file.close();
        jsonBuffer.clear();
    }
    return c;
    SPIFFS.end();
}*/

/*
Channel Configuration::createMosfetChannel(uint8_t id, Mosfet *r)
{

    /*if (SPIFFS.exists(alarmPath))
    {

        ESP_LOGI(TAG, "file exist");
        File file = loadFile(alarmPath);
        JsonObject &root = jsonBuffer.parseObject(file);
        JsonArray &channels = root["Channels"];

        String Channels_0_name = channels[id - 1]["name"].as<String>(); // "Channel1"
        JsonArray &Channels_0_alarms = channels[id - 1]["alarms"];
        Channel c = Channel(Channels_0_name, r, nullptr,id);
        for (int i = 0; i < 14; i++)
        {
            JsonObject &Channels_0_alarms_i = Channels_0_alarms[i];
            uint dow = Channels_0_alarms_i[ALARM_WEEKDAY_KEY];
            uint hour = Channels_0_alarms_i[ALARM_HOUR_KEY];
            uint minute = Channels_0_alarms_i[ALARM_MINUTE_KEY];
            bool state = Channels_0_alarms_i[ALARM_STATE_KEY];
            bool enabled = Channels_0_alarms_i[ALARM_ENABLED_KEY];
            Alarm a = Alarm(dow, hour, minute, state, enabled);
            c.setAlarm(a, i);
        }
        file.close();
        jsonBuffer.clear();
        return c;
    }*/
/*return Channel("", nullptr, nullptr, 0);
}*/

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

void Configuration::setTimezone(long seconds)
{
    _timezoneSeconds = seconds;
}

long Configuration::getTimezone()
{
    return _timezoneSeconds;
}

int Configuration::getDST()
{
    return _DSTseconds;
}

void Configuration::setDST(int seconds)
{
    _DSTseconds = seconds;
}

String Configuration::getChannelNameById(uint8_t id)
{
    return m_channelNames[id - 1];
}

bool Configuration::getFirstTime()
{
    return _firstTime;
}

void Configuration::setFirstTime(bool first_time)
{
    _firstTime = first_time;
}

String Configuration::getEmailServer()
{
    return _email_server;
}

void Configuration::setEmailServer(String server)
{
    _email_server = server;
}

String Configuration::getEmailPort()
{
    return _email_port;
}
void Configuration::setEmailPort(String port)
{
    _email_port = port;
}

String Configuration::getEmailUser()
{
    return _email_user;
}

void Configuration::setEmailUser(String user)
{
    _email_user = user;
}

String Configuration::getEmailPW()
{
    return _email_pw;
}

void Configuration::setEmailPW(String pw)
{
    _email_pw = pw;
}

String Configuration::getEmailRecipient()
{
    return _email_recipient;
}

void Configuration::setEmailRecipient(String recipient)
{
    _email_recipient = recipient;
}

String Configuration::getEmailSubject()
{
    return _email_subject;
}
void Configuration::setEmailSubject(String title)
{
    _email_subject = title;
}

void Configuration::setMosfet1Name(String name)
{
    _mosfet1_name = name;
}

String Configuration::getMosfet1Name()
{
    return _mosfet1_name;
}

void Configuration::setMosfet2Name(String name)
{
    _mosfet2_name = name;
}

String Configuration::getMosfet2Name()
{
    return _mosfet2_name;
}

void Configuration::setInputName(String name)
{
    _name_input = name;
}

String Configuration::getInputName()
{
    return _name_input;
}

void Configuration::setNotificationBoot(bool enable)
{
    _notif_boot = enable;
}

bool Configuration::getNotificationBoot()
{
    return _notif_boot;
}

void Configuration::setNotificationInputChange(bool enable)
{
    _notif_input_change = enable;
}

bool Configuration::getNotificationInputChange()
{
    return _notif_input_change;
}

void Configuration::setNotification_ext_connected(bool enable)
{
    _notif_ext_connected = enable;
}

bool Configuration::getNotification_ext_connected()
{
    return _notif_ext_connected;
}

void Configuration::setNotification_manual_input(bool enable)
{
    _notif_manual_input = enable;
}

bool Configuration::getNotification_manual_input()
{
    return _notif_manual_input;
}

void Configuration::setShedule(uint16_t channel, uint8_t dayOfWeek, uint8_t hour, uint8_t min, bool onOff, bool enable)
{
    _cs[channel].setShedule(dayOfWeek, hour, min, onOff, enable);
}

const char *Configuration::SSID_KEY = "SSID";
const char *Configuration::WIFI_PW_KEY = "WiFi_PW";
const char *Configuration::BOARDNAME_KEY = "BOARDNAME";
const char *Configuration::USERNAME_KEY = "USERNAME";
const char *Configuration::USERPW_KEY = "USERPW";
const char *Configuration::ETH_DHCPEN_KEY = "ETH_DHCPEN";
const char *Configuration::ETH_IPADDR_KEY = "ETH_IPADDR";
const char *Configuration::ETH_GATEWAY_KEY = "ETH_GATEWAY";
const char *Configuration::ETH_SUBNETMASK_KEY = "ETH_SUBNET";
const char *Configuration::ETH_PRIMARYDNS_KEY = "ETH_PRIMARYDNS";
const char *Configuration::ETH_SECONDARYDNS_KEY = "ETH_SECONDARYDNS";
const char *Configuration::WIFI_DHCPEN_KEY = "WIFI_DHCPEN";
const char *Configuration::WIFI_IPADDR_KEY = "WIFI_IPADDR";
const char *Configuration::WIFI_GATEWAY_KEY = "WIFI_GATEWAY";
const char *Configuration::WIFI_SUBNETMASK_KEY = "WIFI_SUBNET";
const char *Configuration::WIFI_PRIMARYDNS_KEY = "WIFI_PRIMARYDNS";
const char *Configuration::WIFI_SECONDARYDNS_KEY = "WIFI_SECONDARYDNS";
const char *Configuration::VERSION_KEY = "VERSION";
const char *Configuration::TIMEZONE_KEY = "TIMEZONE";
const char *Configuration::DST_KEY = "DST";
const char *Configuration::ALARM_WEEKDAY_KEY = "dow";
const char *Configuration::ALARM_HOUR_KEY = "hour";
const char *Configuration::ALARM_MINUTE_KEY = "minute";
const char *Configuration::ALARM_STATE_KEY = "state";
const char *Configuration::ALARM_ENABLED_KEY = "enabled";
const char *Configuration::CHANNEL_NAME_KEY = "name";
const char *Configuration::FIRST_TIME_KEY = "first_time";
const char *Configuration::EMAIL_SERRVER_KEY = "EMAIL_SERVER";
const char *Configuration::EMAIL_PORT_KEY = "EMAIL_PORT";
const char *Configuration::EMAIL_USER_KEY = "EMAIL_USER";
const char *Configuration::EMAIL_PW_KEY = "EMAIL_PW";
const char *Configuration::EMAIL_RECEIVER_KEY = "EMAIL_RECEIVER";
const char *Configuration::EMAIL_TITLE_KEY = "EMAIL_TITLE";
const char *Configuration::NAME_INPUT_KEY = "NAME_INPUT";
const char *Configuration::NAME_MOSFET1_KEY = "NAME_MOSFET1";
const char *Configuration::NAME_MOSFET2_KEY = "NAME_MOSFET2";
const char *Configuration::NOTIF_BOOT_KEY = "NOTIFICATION_BOOT";
const char *Configuration::NOTIF_INPUT_CHANGE_KEY = "NOTIFICATION_INPUT_CHANG";
const char *Configuration::NOTIF_EXT_CONNECT_KEY = "NOTIFICATION_EXT_DIS/CONNECTED";
const char *Configuration::NOTIF_MANUAL_INPUT_KEY = "NOTIFICATION_MANUAL_INPUT";