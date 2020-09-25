#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "relay.hpp"
#include "led.hpp"
#include "alarm.hpp"
class Channel{
    private:
        String m_name;
        Relay* m_relay;
        Led* m_led;  
        uint8_t m_id;

    public: 
        Channel(String name = "",Relay* relay= nullptr,Led* led = nullptr,uint8_t id = 0);
        void loadShedule();
        void startSheduler();
        void turnOff();
        void turnOn();
        void toggle();
        void setName(String name);
        String getName();
        void activateTimer(uint16_t time);
        void activatePulse(uint64_t time);
        void setAlarm(Alarm alarm,uint8_t index);
        Alarm* getAlarm(uint8_t index);
        bool getState();
       
        void updateLed();
        void toggleLed();
        void setLed(bool state);
        
};
#endif