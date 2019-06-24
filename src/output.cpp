#include "output.hpp"

#include "IO.hpp"
#include <list>
#include <iterator>

extern "C"
{
 

    
}
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
    startSheduler();
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
    time *= 1000;
    m_pulseTime = (time <= 60000000) ? time : 60000000;
}

void Output::activatePulse()
{
    toggle();
    activatePulse(m_pulseTime);
}

void Output::activatePulse(uint64_t time)
{
    
}

void Output::setTimerTime(uint16_t time)
{
    m_timerTime = (time <= 720) ? time : 720;
}

void Output::activateTimer()
{
    toggle();
    activateTimer(m_timerTime);
}

void Output::activateTimer(uint64_t time)
{

}

void Output::startSheduler()
{
    
}
