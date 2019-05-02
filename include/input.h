#ifndef INPUT_H
#define INPUT_H

#include "TCA6424A.h"

class Input
{
  private:
    bool m_state;
    bool m_isExtension;
    uint16_t m_pin;
    void updateTCA();
    TCA6424A * m_tca;
    void initPin();
    uint8_t m_id;
    void readTCA();
  public:
    Input(uint8_t id,uint16_t pin,bool isExtension,TCA6424A* tca);
    uint8_t getId();
    bool read();
    ~Input();

};
#endif