#include "ChannelShedule.hpp"
#include "global.hpp"

ChannelShedule::ChannelShedule(RelayChannel *channel,uint16_t id)
{
    _channelID = id;
    _channel = channel;
}

void ChannelShedule::setChannelID(uint16_t id)
{
    _channelID = id;
}

void ChannelShedule::setChannel(RelayChannel *channel)
{
    _channel = channel;
}

void ChannelShedule::setShedule(uint8_t dayOfWeek, uint8_t hour, uint8_t minute, bool onOff, bool enable)
{
    uint8_t index = dayOfWeek; //+ (onOff ? 0 : 7); //first 7 are start times last 7 stop times
    uint8_t offset = onOff ? 0 : 7;
    index += offset;
    //_shedules[index].dateTime.tm_wday = dayOfWeek;
    //_shedules[index].dateTime.tm_hour = hour;
    //_shedules[index].dateTime.tm_min = minute;
    _shedules[index].dow = dayOfWeek;
    _shedules[index].hour = hour;
    _shedules[index].minute = minute;
    _shedules[index].onOff = onOff;
    _shedules[index].enable = enable;
}

void ChannelShedule::Update(tm *time)
{
    uint8_t index = time->tm_wday;
    Shedule *start = _shedules + index;
    Shedule *stop = _shedules + index + 7;
    if (start->hour == time->tm_hour && start->minute == time->tm_min && start->enable)
    {
        if (!_isStartTriggered)
        {
            _isStartTriggered = true;
            if (_channel != nullptr)
            {
                mm.getModuleFromChannelID(_channelID)->Activate();
                _channel->turnOn();
                mm.getModuleFromChannelID(_channelID)->Disactivate();
            }
        }
    }
    else if (stop->hour == time->tm_hour && stop->minute == time->tm_min && stop->enable)
    {
        if (!_isStopTriggered)
        {
            _isStopTriggered = true;
            if (_channel != nullptr)
            {
                mm.getModuleFromChannelID(_channelID)->Activate();
                _channel->turnOff();
                mm.getModuleFromChannelID(_channelID)->Disactivate();
            }
        }
    }
    else
    {
        _isStartTriggered = false;
        _isStopTriggered = false;
    }
}

Shedule *ChannelShedule::getShedule(uint8_t dow, uint8_t onOff)
{
    uint8_t index = dow + (onOff ? 0 : 7); //Correct
    return &_shedules[index];
}