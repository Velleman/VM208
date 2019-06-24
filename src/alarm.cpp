#include "alarm.hpp"

Alarm::Alarm(uint8_t weekday, uint8_t hour, uint8_t minute, bool state,bool isEnabled)
{
}

void Alarm::setHour(uint8_t hour)
{
    if (hour < 24)
        m_hour = hour;
}

uint8_t Alarm::getHour()
{
    return m_hour;
}

void Alarm::setMinute(uint8_t minute)
{
    if (minute < 60)
        m_minute = minute;
}

uint8_t Alarm::getMinute()
{
    return m_minute;
}

void Alarm::setWeekday(uint8_t weekday)
{
    if (weekday < 8)
    {
        m_weekday = weekday;
    }
}

uint8_t Alarm::getWeekday()
{
    return m_weekday;
}

bool Alarm::getState()
{
    return m_state;
}

void Alarm::setState(bool state)
{
    m_state = state;
}

void Alarm::setEnabled(bool state)
{
    m_enabled = state;
}

bool Alarm::isEnabled()
{
    return m_enabled;
}