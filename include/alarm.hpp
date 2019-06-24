#ifndef ALARM_HPP
#define ALARM_HPP

#include <stdint.h>

class Alarm{
    private:
        uint8_t m_weekday;
        uint8_t m_hour;
        uint8_t m_minute;
        bool m_state;
        bool m_enabled;
    public: 
        Alarm(uint8_t weekday = 0,uint8_t hour = 0,uint8_t minute = 0,bool state = false,bool isEnabled = false);

        void setWeekday(uint8_t weekday);
        uint8_t getWeekday();

        void setHour(uint8_t hour);
        uint8_t getHour();

        void setMinute(uint8_t minute);
        uint8_t getMinute(); 

        void setState(bool state);
        bool getState();

        void setEnabled(bool enabled);
        bool isEnabled();

};
#endif