#include "input.h"

Input::Input(uint8_t id,uint16_t pin,bool isExtension,TCA6424A *tca):
m_id(id),
m_pin(pin),
m_isExtension(isExtension),
m_tca(tca)
{
    initPin();
}

uint8_t Input::getId(){
    return m_id;
}

void Input::initPin(){
    
}

void Input::readTCA(){
    m_state = m_tca->readPin(m_pin);
}

void Input::initPin(){
    m_tca->setPinDirection(m_pin,TCA6424A_INPUT);
}