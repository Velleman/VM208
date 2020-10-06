#include "PulseAndTimer.hpp"
#include "Arduino.h"
#include "global.hpp"
#include "VM208INT.h"
#include "IO.hpp"
#include "RelayChannel.h"

void timerTask(void *arg)
{
    timerParam_t timers[268];
    TimeParameters_t param;
    timerStatus_t status;
    long relay;
    const uint16_t timerPeriodCheck = 100;
    unsigned long lastTimerCheck = 0;
    for (int i = 0; i < 268; i++)
    {
        timers[i].enabled = false;
        timers[i].endTime = 0;
        status.status[i] = timers[i].enabled;
    }
    while (true)
    {
        if (xQueueReceive(timerQueue, &param, 0))
        {
            //handle param
            auto channelId = param.id;
            auto time = param.time;
            time *= 60000;
            timers[channelId - 1].enabled = true;
            timers[channelId - 1].endTime = millis() + time;
            RelayChannel *channel = mm.getChannel(channelId);
            if (channel != nullptr)
            {
                unsigned long startTime = millis();
                xSemaphoreTake(g_Mutex, portMAX_DELAY);
                channel->toggle();
                xSemaphoreGive(g_Mutex);
            }
            for (int i = 0; i < 268; i++)
            {
                status.status[i] = timers[i].enabled;
            }
            xQueueOverwrite(timerStatusQueue, &status);
        }
        if (xQueueReceive(timerStopQueue, &relay, 0))
        {
            timers[relay -1].enabled = false;
            for (int i = 0; i < 268; i++)
            {
                status.status[i] = timers[i].enabled;
            }
            xQueueOverwrite(timerStatusQueue, &status);
        }
        if (millis() - lastTimerCheck > timerPeriodCheck)
        {
            for (int i = 0; i < 268; i++)
            {
                if (timers[i].enabled && timers[i].endTime < millis())
                {
                    xSemaphoreTake(g_Mutex, portMAX_DELAY);
                    mm.getChannel(i + 1)->toggle();
                    xSemaphoreGive(g_Mutex);
                    timers[i].enabled = false;
                    for (int i = 0; i < 268; i++)
                    {
                        status.status[i] = timers[i].enabled;
                    }
                    xQueueOverwrite(timerStatusQueue, &status);
                }
            }
            lastTimerCheck = millis();
        }
        delay(10);
    }

    vTaskDelete(NULL);
}

void pulseTask(void *arg)
{
    timerParam_t timers[268];
    TimeParameters_t param;
    pulseStatus_t status;
    long relay;
    const uint16_t timerPeriodCheck = 10;
    unsigned long lastTimerCheck = 0;
    for (int i = 0; i < 268; i++)
    {
        timers[i].enabled = false;
        timers[i].endTime = 0;
        status.status[i] = timers[i].enabled;
    }
    while (true)
    {
        if (xQueueReceive(pulseQueue, &param, 0))
        {
            //handle param
            auto channelId = param.id;
            auto time = param.time;
            timers[channelId - 1].enabled = true;
            timers[channelId - 1].endTime = millis() + time;
            RelayChannel *channel = mm.getChannel(channelId);
            if (channel != nullptr)
            {
                xSemaphoreTake(g_Mutex, portMAX_DELAY);
                channel->toggle();
                xSemaphoreGive(g_Mutex);
            }
            for (int i = 0; i < 268; i++)
            {
                status.status[i] = timers[i].enabled;
            }
            xQueueOverwrite(pulseStatusQueue, &status);
        }
        if (xQueueReceive(pulseStopQueue, &relay, 0))
        {
            timers[relay - 1].enabled = false;
            for (int i = 0; i < 268; i++)
            {
                status.status[i] = timers[i].enabled;
            }
            xQueueOverwrite(pulseStatusQueue, &status);
        }
        if (millis() - lastTimerCheck > timerPeriodCheck)
        {
            for (int i = 0; i < 268; i++)
            {
                if (timers[i].enabled && timers[i].endTime < millis())
                {
                    xSemaphoreTake(g_Mutex, portMAX_DELAY);
                    mm.getChannel(i + 1)->toggle();
                    xSemaphoreGive(g_Mutex);
                    timers[i].enabled = false;
                    for (int i = 0; i < 268; i++)
                    {
                        status.status[i] = timers[i].enabled;
                    }
                    xQueueOverwrite(pulseStatusQueue, &status);
                }
            }
            lastTimerCheck = millis();
        }

        delay(10);
    }

    vTaskDelete(NULL);
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