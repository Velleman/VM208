#ifndef RELAY_H
#define RELAY_H

#include "stdint.h"
#include "time.h"
#include "TCA6424A.h"
class Relay
{
  private:
    bool m_state;
    bool m_isExtension;
    uint16_t m_pin;
    void updateTCA();
    TCA6424A * m_tca;
    void initPin();
    uint8_t m_id;
  public:
    Relay(uint8_t id,uint16_t pin,bool isExtension,bool initState,TCA6424A* tca);
    
    uint8_t getId();
    void turnOn(void);
    void turnOff(void);

    void setPulse(uint16_t time);

    void setTimer(uint16_t time);

    void setSchedule(tm start_date,tm end_date);

    ~Relay();

};
#endif