#include "input.hpp"

Input::Input(uint8_t id, uint16_t pin, TCA6424A_TS *tca) : m_id(id),
                                                        m_pin(pin),
                                                        m_tca(tca)
{
    initPin();
    m_state = false;
}

uint8_t Input::getId()
{
    return m_id;
}

bool Input::read()
{
    readTCA();
    return m_state;
}

void Input::readTCA()
{
    if (m_tca != nullptr)
        m_state = m_tca->readPin(m_pin);
}

void Input::initPin()
{
    if (m_tca != nullptr)
        m_tca->setPinDirection(m_pin, TCA6424A_INPUT);
}