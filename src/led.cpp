#include "led.hpp"

void Led::turnOff()
{
    m_state = true;
    updateTCA();
}

void Led::turnOn()
{
    m_state = false;
    updateTCA();
}

void Led::reflect()
{
    updateTCA();
}