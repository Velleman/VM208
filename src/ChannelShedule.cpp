#include "ChannelShedule.hpp"

ChannelShedule::ChannelShedule(RelayChannel *channel)
{
    _channel = channel;
}

void ChannelShedule::setShedule(uint8_t dayOfWeek, uint8_t hour, uint8_t minute, bool onOff, bool enable)
{
    uint8_t index = dayOfWeek; //+ (onOff ? 0 : 7); //first 7 are start times last 7 stop times
    uint8_t offset = onOff ? 0 : 7;
    index += offset;
    Serial.printf("Offset is: %d\r\n",index);
    _shedules[index].dateTime.tm_wday = dayOfWeek;
    _shedules[index].dateTime.tm_hour = hour;
    _shedules[index].dateTime.tm_min = minute;
    _shedules[index].onOff = onOff;
    _shedules[index].enable = enable;
}

void ChannelShedule::Update(tm *time)
{
    uint8_t index = time->tm_wday;
    Shedule *start = _shedules + index;
    Shedule *stop = _shedules + index + 7;
    if (start->dateTime.tm_hour == time->tm_hour && start->dateTime.tm_min == time->tm_min && start->enable)
    {
        if (!_isStartTriggered)
        {
            _isStartTriggered = true;
            _channel->turnOn();
        }
    }
    else if (stop->dateTime.tm_hour == time->tm_hour && stop->dateTime.tm_min == time->tm_min && stop->enable)
    {
        if (!_isStopTriggered)
        {
            _isStopTriggered = true;
            _channel->turnOff();
        }
    }
    else
    {
        _isStartTriggered = false;
        _isStopTriggered = false;
    }
}

Shedule* ChannelShedule::getShedule(uint8_t dow,uint8_t onOff)
{
    uint8_t index = dow + (onOff ? 0 : 7); //Correct
    return &_shedules[index];
}