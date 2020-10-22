#pragma once
#include "RelayChannel.h"
#include "time.h"
struct Shedule{
    //tm dateTime;
    uint8_t dow;
    uint8_t hour;
    uint8_t minute;
    bool onOff;
    bool enable;
};
class ChannelShedule{
public:
    ChannelShedule(RelayChannel* channel=nullptr,uint16_t channelID=0);
    void setShedule(uint8_t dayOfWeek,uint8_t hour,uint8_t minute,bool onOff,bool enable);
    void Update(tm* time);
    uint8_t getDoW();
    Shedule* getShedule(uint8_t dow,uint8_t onOff);
    void setChannel(RelayChannel* channel);
    void setChannelID(uint16_t id);
private:
    uint16_t _channelID;
    Shedule _shedules[14];
    RelayChannel* _channel;
    tm* _timeAndDate;
    bool _isStartTriggered;
    bool _isStopTriggered;
};
