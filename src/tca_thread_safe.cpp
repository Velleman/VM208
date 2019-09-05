#include "tca_thread_safe.hpp"
#include "freertos/FreeRTOS.h"
#include "global.hpp"

TCA6424A_TS::TCA6424A_TS() : TCA6424A(TCA6424A_DEFAULT_ADDRESS)
{
    devAddr = TCA6424A_DEFAULT_ADDRESS;
    
    registers[4] = 0xFF;
    registers[5] = 0xFF;
    registers[6] = 0xFF;
    
    registers[0x0C] = 0xFF;
    registers[0x0D] = 0xFF;
    registers[0x0E] = 0xFF;
}

TCA6424A_TS::TCA6424A_TS(uint8_t address) : TCA6424A(address)
{
    devAddr = address;
    registers[4] = 0xFF;
    registers[5] = 0xFF;
    registers[6] = 0xFF;
    
    registers[0x0C] = 0xFF;
    registers[0x0D] = 0xFF;
    registers[0x0E] = 0xFF;
}

bool TCA6424A_TS::ts_testConnection()
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    bool isConnected = false;
    int8_t result = I2Cdev::readBytes(devAddr, TCA6424A_RA_INPUT0, 3, buffer);
    isConnected = (bool)(result == 3);
    xSemaphoreGive(g_Mutex);
    return isConnected;
}

bool TCA6424A_TS::ts_readPin(uint16_t pin)
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    bool result;
    I2Cdev::readBit(devAddr, TCA6424A_RA_INPUT0 + (pin / 8), pin % 8, buffer);
    result = buffer[0];
    xSemaphoreGive(g_Mutex);
    return result;
}

void TCA6424A_TS::ts_setPinDirection(uint16_t pin, bool direction)
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    uint8_t pinNumber = pin%8;
    uint8_t reg = TCA6424A_RA_CONFIG0 + (pin / 8);
    if(direction)
    {
        registers[reg] |= (0x01 << pinNumber);
    }else{
        registers[reg] &= ~(0x01 << pinNumber);
    }
    I2Cdev::writeByte(devAddr, reg, registers[reg]);
    xSemaphoreGive(g_Mutex);
}

void TCA6424A_TS::ts_writePin(uint16_t pin, bool polarity)
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    uint8_t pinNumber = pin%8;
    uint8_t reg = TCA6424A_RA_OUTPUT0 + (pin / 8);
    if(polarity)
    {
        registers[reg] |= (0x01 << pinNumber);
    }else{
        registers[reg] &= ~(0x01 << pinNumber);
    }
    I2Cdev::writeByte(devAddr, TCA6424A_RA_OUTPUT0 + (pin / 8), registers[reg]);
    
    xSemaphoreGive(g_Mutex);
}

void TCA6424A_TS::ts_writeByte(uint8_t reg, uint8_t data)
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    I2Cdev::writeByte(devAddr, reg, data);
    xSemaphoreGive(g_Mutex);
}

uint8_t TCA6424A_TS::ts_readBank(uint8_t reg)
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    I2Cdev::readByte(devAddr,reg, buffer);
    xSemaphoreGive(g_Mutex);
    return buffer[0];
    
}