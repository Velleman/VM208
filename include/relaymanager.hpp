#ifndef RELAYMANAGER_H
#define RELAYMANAGER_H
#include "relay.hpp"

class RelayManager{
    private:
        Relay* m_relays;
    public:
        RelayManager(Relay* relays);
        Relay* getRelayById(uint8_t id);
        Relay* getRelayById(String id);
        Relay* getRelay(uint8_t index);
        Relay  operator[](uint8_t i);
};

#endif