#ifndef LED_H
#define LED_H
#include "output.hpp"
class Led : public Output
{
  private:
    
  public:
    Led(uint8_t id = 0,uint16_t pin = 0,bool initState = 0,TCA6424A_TS* tca=nullptr):Output(id,pin,initState,tca){}
    void turnOff();
    void turnOn();

};
#endif