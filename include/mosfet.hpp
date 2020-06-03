#ifndef MOSFET_H
#define MOSFET_H
#include "output.hpp"

class Mosfet : public Output
{
  private:
    
  public:
    Mosfet(uint8_t id=0,uint16_t pin=0,bool initState = false,TCA6424A_TS* tca = nullptr,bool setState=true):Output(id,pin,initState,tca,setState){}
};
#endif