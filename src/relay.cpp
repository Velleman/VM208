#include "relay.h"

Relay::Relay(uint8_t id,uint16_t pin,bool isExtension,bool initState,TCA6424A *tca):
        m_id(id),
        m_pin(pin),
        m_isExtension(isExtension),
        m_state(initState),
        m_tca(tca){
    initPin();
    
}

uint8_t Relay::getId(){
    return m_id;
}

void Relay::turnOff(){
    m_state = false;
    updateTCA();
}

void Relay::turnOn(){
    m_state = true;
    updateTCA();
}

void Relay::updateTCA(){
    m_tca->writePin(m_pin,m_state);
}

void Relay::initPin(){
    m_tca->setPinDirection(m_pin,TCA6424A_OUTPUT);
}