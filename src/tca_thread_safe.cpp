#include "tca_thread_safe.hpp"
#include "FreeRTOS.h"
#include "global.hpp"

TCA6424A_TS::TCA6424A_TS() : TCA6424A(TCA6424A_DEFAULT_ADDRESS)
{
    devAddr = TCA6424A_DEFAULT_ADDRESS;
}

TCA6424A_TS::TCA6424A_TS(uint8_t address) : TCA6424A(TCA6424A_DEFAULT_ADDRESS)
{
    devAddr = address;
}

bool TCA6424A_TS::testConnection()
{
    bool isConnected = false;
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    int8_t result = I2Cdev::readBytes(devAddr, TCA6424A_RA_INPUT0, 3, buffer);
    if(result == -1)
    {
        Serial.println("FAIL READ BYTES");
        result = I2Cdev::readBytes(devAddr, TCA6424A_RA_INPUT0, 3, buffer);
    }
    isConnected = (bool)(result == 3);
    xSemaphoreGive(g_Mutex);
    return isConnected;
}

bool TCA6424A_TS::readPin(uint16_t pin)
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);

    bool success = I2Cdev::readBit(devAddr, TCA6424A_RA_INPUT0 + (pin / 8), pin % 8, buffer);
    if(!success)
    {
        Serial.println("FAIL READ BIT");
        I2Cdev::readBit(devAddr, TCA6424A_RA_INPUT0 + (pin / 8), pin % 8, buffer);
    }
    xSemaphoreGive(g_Mutex);
    return buffer[0];
}

void TCA6424A_TS::setPinDirection(uint16_t pin, bool direction)
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    I2Cdev::writeBit(devAddr, TCA6424A_RA_CONFIG0 + (pin / 8), pin % 8, direction);
    xSemaphoreGive(g_Mutex);
}

void TCA6424A_TS::writePin(uint16_t pin, bool polarity)
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    bool success = I2Cdev::writeBit(devAddr, TCA6424A_RA_OUTPUT0 + (pin / 8), pin % 8, polarity);
    if(!success)
    {
        Serial.println("FAIL WRITE BIT");
        I2Cdev::writeBit(devAddr, TCA6424A_RA_OUTPUT0 + (pin / 8), pin % 8, polarity);
    }
    xSemaphoreGive(g_Mutex);
}

void TCA6424A_TS::writeByte(uint8_t reg, uint8_t data)
{
    xSemaphoreTake(g_Mutex, portMAX_DELAY);
    bool success = I2Cdev::writeByte(devAddr, reg, data);
    if(!success)
    {
        Serial.println("FAIL WRITE BIT");
        bool success = I2Cdev::writeByte(devAddr, reg, data);
    }
    xSemaphoreGive(g_Mutex);
}