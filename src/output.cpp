#include "output.hpp"

Output::Output(uint8_t id, uint16_t pin, bool initState, TCA6424A_TS *tca) : m_id(id),
                                                                          m_pin(pin),
                                                                          m_state(initState),
                                                                          m_tca(tca),
                                                                          m_isAccessible(false),
                                                                          m_pulseTime(5),
                                                                          m_timerTime(1)
{
    initPin();
    updateTCA();
}

uint8_t Output::getId()
{
    return m_id;
}

void Output::turnOff()
{
    m_state = false;
    updateTCA();
}

void Output::turnOn()
{
    m_state = true;
    updateTCA();
}

void Output::setState(bool state)
{
    m_state = state;
    updateTCA();
}

void Output::updateTCA()
{
    if (m_tca != nullptr && m_isAccessible)
    {
        m_tca->writePin(m_pin, m_state);
    }
}

void Output::toggle()
{
    m_state = !m_state;
    updateTCA();
}

void Output::initPin()
{
    if (m_tca != nullptr)
    {
        if (m_tca->testConnection())
        {
            m_isAccessible = true;
            m_tca->setPinDirection(m_pin, TCA6424A_OUTPUT);
        }
        else
        {
            m_isAccessible = false;
        }
    }
}

bool Output::getState()
{
    return m_state;
}

void Output::setPulseTime(uint16_t time)
{
    m_pulseTime = (time <= 6000) ? time : 6000;
}

void Output::activatePulse()
{
    toggle();
    delay(m_pulseTime);
    toggle();
}

void Output::activatePulse(uint16_t time)
{
    uint16_t pulseTime = (time <= 6000) ? time : 6000;
    toggle();
    delay(pulseTime);
    toggle();
}

void Output::setTimerTime(uint16_t time)
{
    m_timerTime = (time <= 720) ? time : 720;
}

void Output::activateTimer()
{
    unsigned long time = m_timerTime*60000;
    toggle();
    delay(time);
    toggle();
}


void Output::activateTimer(uint16_t time)
{
    unsigned long time_ms = time*60000; //1 = 1min => 60000ms;
    toggle();
    delay(time_ms);
    toggle();
}