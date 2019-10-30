#include "output.hpp"

#include "vm208_io.hpp"
#include <list>
#include <iterator>

extern "C"
{
}
Output::Output(uint8_t id, uint16_t pin, bool initState, TCA6424A_TS *tca, bool setState) : m_id(id),
                                                                                            m_pin(pin),
                                                                                            m_state(initState),
                                                                                            m_tca(tca),
                                                                                            m_isAccessible(false),
                                                                                            m_pulseTime(5),
                                                                                            m_timerTime(1)
{
    m_state = initState;
    m_setState = setState;
    if (m_setState == true)
    {
        initPin(true);
    }else{
        m_isAccessible = true;
    }
    m_setState = true;
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
        if (m_setState)
        {
            m_tca->ts_writePin(m_pin, m_state);
        }
    }
}

void Output::toggle()
{
    m_state = !m_state;
    updateTCA();
}

void Output::initPin(bool checkConnection)
{
    if (m_tca != nullptr)
    {
        if (checkConnection)
        {
            if (m_tca->testConnection())
            {
                m_isAccessible = true;
                m_tca->ts_setPinDirection(m_pin, TCA6424A_OUTPUT);
                updateTCA();
            }
            else
            {
                m_isAccessible = false;
            }
        }
        else
        {
            m_isAccessible = true;
            m_tca->ts_setPinDirection(m_pin, TCA6424A_OUTPUT);
            updateTCA();
        }
    }
}

bool Output::getState()
{
    return m_state;
}