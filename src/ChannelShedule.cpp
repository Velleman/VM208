#include "ChannelShedule.hpp"

ChannelShedule::ChannelShedule(RelayChannel *channel)
{
    _channel = channel;
}

void ChannelShedule::setShedule(uint8_t dayOfWeek, uint8_t hour, uint8_t minute, bool onOff, bool enable)
{
    uint8_t index = dayOfWeek + onOff ? 0 : 7; //first 7 are start shedule last 7 stop shedule
    shedules[index].dateTime.tm_wday = dayOfWeek;
    shedules[index].dateTime.tm_hour = hour;
    shedules[index].dateTime.tm_min = minute;
    shedules[index].onOff = onOff;
    shedules[index].enable = enable;
}

void ChannelShedule::Update(tm *time)
{
    uint8_t index = time->tm_wday;
    Shedule *start = shedules + index;
    Shedule *stop = shedules + index + 7;
    if (start->dateTime.tm_hour == time->tm_hour && start->dateTime.tm_min == time->tm_min && start->enable)
    {
        if (!_isTriggered)
        {
            _isTriggered = true;
            _channel->turnOn();
        }
    }
    else if (stop->dateTime.tm_hour == time->tm_hour && stop->dateTime.tm_min == time->tm_min && stop->enable)
    {
        if (!_isTriggered)
        {
            _isTriggered = true;
            _channel->turnOff();
        }
    }
    else
    {
        _isTriggered = false;
    }
}