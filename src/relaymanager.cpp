#include "relaymanager.hpp"

RelayManager::RelayManager(Relay* relays):m_relays(relays){
}

Relay RelayManager::operator[](uint8_t i){
    if(i < 12)
    {
        return m_relays[i];
    }
    else
    {
        return m_relays[0];
    }
    
}