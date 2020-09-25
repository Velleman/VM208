#include "channel.hpp"
#include "IO.hpp"
#include "esp_err.h"
#include "esp_timer.h"
#include "global.hpp"
#include "config_vm208.hpp"
#include <list>
/*extern "C"
{

    static void timerTask(void *arg)
    {
        uint32_t id = (uint32_t)arg;
        Channel *c = getChannelById(id);
        unsigned long startTime = millis();
        if (c != nullptr)
        {
            c->toggle();
            uint64_t timerTime = c->getTimerTime();
            timerTime /= 1000; // go to miliseconds;
            while ((millis() - startTime) < timerTime)
            {
                c->toggleLed();
                delay(1000);
            }
            c->toggle();
        }
        c->clearTimer();
        vTaskDelete( NULL );
    }

    static void pulseTask(void *arg)
    {
        uint32_t id = (uint32_t)arg;
        Channel *c = getChannelById(id);
        unsigned long startTime = millis();
        if (c != nullptr)
        {
            c->toggle();
            uint64_t pulseTime = c->getPulseTime();
            pulseTime /= 1000; // go to miliseconds;
            while ((millis() - startTime) < pulseTime)
            {
                c->toggleLed();
                delay(50);
            }
            c->toggle();
        }
        c->clearPulse();
        vTaskDelete( NULL );
    }
}

Channel::Channel(String name, Relay *relay, Led *led, uint8_t id) : m_name(name), m_relay(relay), m_led(led), m_id(id), m_isTimerActive(false), m_isPulseActive(false)
{ 
}

void Channel::loadShedule()
{
}

void Channel::startSheduler()
{
}

void Channel::turnOn()
{
    m_relay->turnOn();
    if (m_led != nullptr)
        m_led->turnOn();
}

void Channel::turnOff()
{
    m_relay->turnOff();
    if (m_led != nullptr)
        m_led->turnOff();
}

void Channel::toggle()
{
    m_relay->toggle();
    if (m_led != nullptr)
        m_relay->getState() ? m_led->turnOn() : m_led->turnOff();
}

void Channel::setName(String name)
{
    m_name = name;
}

String Channel::getName()
{
    return m_name;
}




bool Channel::getState()
{
    return m_relay->getState();
}



void Channel::updateLed()
{
    m_relay->getState() ? m_led->turnOn() : m_led->turnOff();
}

void Channel::toggleLed()
{
    if (m_led != nullptr)
        m_led->toggle();
}

void Channel::setLed(bool state)
{
    state ? m_led->turnOn() : m_led->turnOff();
}

*/