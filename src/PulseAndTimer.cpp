#include "PulseAndTimer.hpp"
#include "Arduino.h"
//#include "global.hpp"
#include "VM208INT.h"
#include "IO.hpp"
#include "RelayChannel.h"
extern "C"
{

    static void timerTask(void *arg)
    {
        TimeParameters_t* params = (TimeParameters_t*)arg;
        auto channelId = params->id;
        auto time = params->time;
        unsigned long startTime = millis();
        RelayChannel* channel = mm.getChannel(channelId);
        if (channel != nullptr)
        {
            channel->toggle();
            while ((millis() - startTime) < time)
            {
                channel->toggleLed();
                delay(1000);
            }
            channel->toggle();
        }
        vTaskDelete( NULL );
    }

    static void pulseTask(void *arg)
    {
        TimeParameters_t* params = (TimeParameters_t*)arg;
        auto channelId = params->id;
        auto time = params->time;
        unsigned long startTime = millis();
        RelayChannel* channel = mm.getChannel(channelId);
        if (channel != nullptr)
        {
            channel->toggle();
            
            //time /= 1000; // go to miliseconds;
            while ((millis() - startTime) < time)
            {
                channel->toggleLed();
                delay(50);
            }
            channel->toggle();
        }
        vTaskDelete( NULL );
    }
}
/*
uint64_t VM208TimeChannel::getPulseTime()
{
    return m_pulseTime;
}

uint64_t VM208TimeChannel::getTimerTime()
{
    return m_timerTime;
}

void VM208TimeChannel::activateTimer(uint16_t timertime)
{
    unsigned long time_us = timertime * 60000000UL; //1 = 1min => 60000ms;
    //Serial.print("Pulse Time:");
    //Serial.println(time_us);
    m_timerTime = time_us;
    if (!m_isTimerActive)
    {
        xTaskCreate(timerTask, "timer", 4092, (void *)_id, (tskIDLE_PRIORITY + 2), &timerTaskHandle);
        m_isTimerActive = true;
    }
}

void VM208TimeChannel::activatePulse(uint64_t pulsetime)
{

    uint32_t t = ((uint32_t)pulsetime) * ((uint32_t)1000);
    uint32_t pulseTime = (t <= 60000000) ? t : 60000000;
    m_pulseTime = pulseTime;
    if (!m_isPulseActive)
    {
        xTaskCreate(pulseTask, "pulse", 4092, (void *)_id, (tskIDLE_PRIORITY + 2), &pulseTaskHandle);
        m_isPulseActive = true;
    }
}

void VM208TimeChannel::setAlarm(Alarm a, uint8_t index)
{
    if (index < 14)
    {
        alarms[index] = a;
    }
    else
    {
        Serial.println("Alarm index is invalid");
    }
}

Alarm *VM208TimeChannel::getAlarm(uint8_t index)
{
    return alarms + index;
}

void VM208TimeChannel::clearTimerAndPulse()
{
    clearPulse();
    clearTimer();
}

void VM208TimeChannel::clearPulse()
{
    if (m_isPulseActive)
    {
        m_isPulseActive = false;
        vTaskDelete(pulseTaskHandle);
    }
}

void VM208TimeChannel::clearTimer()
{
    if (m_isTimerActive)
    {
        m_isTimerActive = false;
        vTaskDelete(timerTaskHandle);
    }
}

void VM208TimeChannel::disableSheduler()
{
    for (int i = 0; i < 14; i++)
    {
        alarms[i].setEnabled(false);
    }
}

bool VM208TimeChannel::isSheduleActive()
{
    for (int i = 0; i < 14; i++)
    {
        if (alarms[i].isEnabled())
            return true;
    }
    return false;
}

bool VM208TimeChannel::isTimerActive()
{
    return m_isTimerActive;
}

bool VM208TimeChannel::isPulseActive()
{
    return m_isPulseActive;
}*/