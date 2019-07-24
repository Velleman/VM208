#ifndef RELAY_H
#define RELAY_H
#include "output.hpp"

class Relay : public Output
{
private:
public:
  Relay(uint8_t id = 0, uint16_t pin = 0, bool initState = false, TCA6424A_TS *tca = nullptr) : Output(id, pin, initState, tca) {}

};
#endif