#ifndef TCA_THREAD_SAFE_H
#define TCA_THREAD_SAFE_H
#include "TCA6424A.h"
class TCA6424A_TS : public TCA6424A
{
private:
    uint8_t devAddr;
    uint8_t buffer[3];

public:
    TCA6424A_TS();
    TCA6424A_TS(uint8_t address);
    bool ts_testConnection();
    bool ts_readPin(uint16_t pin);
    void ts_setPinDirection(uint16_t pin, bool direction);
    void ts_writePin(uint16_t pin, bool polarity);
    void ts_writeByte(uint8_t register, uint8_t data);
};
#endif