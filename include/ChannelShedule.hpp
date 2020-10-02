#pragma once
#include "RelayChannel.h"
#include "time.h"
struct Shedule{
    tm dateTime;
    bool onOff;
    bool enable;
};
class ChannelShedule{
public:
    ChannelShedule(RelayChannel* channel);
    void setShedule(uint8_t dayOfWeek,uint8_t hour,uint8_t minute,bool onOff,bool enable);
    void Update(tm* time);
private:
    Shedule shedules[14];
    RelayChannel* _channel;
    tm* _timeAndDate;
    bool _isTriggered;
};
