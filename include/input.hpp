#ifndef INPUT_H
#define INPUT_H

#include "tca_thread_safe.hpp"

class Input
{
  private:
    bool m_state;
    uint8_t m_id;
    uint16_t m_pin;
    TCA6424A_TS * m_tca;
    void updateTCA(void);    
      
    void readTCA();
  public:
    Input(uint8_t id = 0,uint16_t pin = 0,TCA6424A_TS* tca = nullptr);
    uint8_t getId();
    bool read();
    void initPin(void);  

};
#endif