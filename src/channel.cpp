#include "channel.hpp"
#include "IO.hpp"
#include "esp_err.h"
#include "esp_timer.h"
#include "global.hpp"
#include "config_vm208.hpp"

extern "C"
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
/**
 * returns the time in microseconds
 */
uint64_t Channel::getPulseTime()
{
    return m_pulseTime;
}
/**
 * returns the time in microseconds
 */
uint64_t Channel::getTimerTime()
{
    return m_timerTime;
}

void Channel::activateTimer(uint16_t timertime)
{
    unsigned long time_us = timertime * 60000000UL; //1 = 1min => 60000ms;
    //Serial.print("Pulse Time:");
    //Serial.println(time_us);
    m_timerTime = time_us;
    if (!m_isTimerActive)
    {
        xTaskCreate(timerTask, "timer", 2048, (void *)m_id, (tskIDLE_PRIORITY + 2), &timerTaskHandle);
        m_isTimerActive = true;
    }
}

void Channel::activatePulse(uint64_t pulsetime)
{

    uint32_t t = ((uint32_t)pulsetime) * ((uint32_t)1000);
    uint32_t pulseTime = (t <= 60000000) ? t : 60000000;
    m_pulseTime = pulseTime;
    if (!m_isPulseActive)
    {
        xTaskCreate(pulseTask, "pulse", 2048, (void *)m_id, (tskIDLE_PRIORITY + 2), &pulseTaskHandle);
        m_isPulseActive = true;
    }
}

void Channel::setAlarm(Alarm a, uint8_t index)
{
    if (index < 14)
    {
        alarms[index] = a;
    }
}

Alarm *Channel::getAlarm(uint8_t index)
{
    return alarms + index;
}

bool Channel::getState()
{
    return m_relay->getState();
}

bool Channel::isTimerActive()
{
    return m_isTimerActive;
}

bool Channel::isPulseActive()
{
    return m_isPulseActive;
}

void Channel::updateLed()
{
    m_relay->getState() ? m_led->turnOn() : m_led->turnOff();
}

void Channel::toggleLed()
{
    m_led->toggle();
}

void Channel::clearTimerAndPulse()
{
    clearPulse();
    clearTimer();
}

void Channel::clearPulse()
{
    if (m_isPulseActive)
    {
        m_isPulseActive = false;
        vTaskDelete(pulseTaskHandle);
    }
}

void Channel::clearTimer()
{
    if (m_isTimerActive)
    {
        m_isTimerActive = false;
        vTaskDelete(timerTaskHandle);
    }
}