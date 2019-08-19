/* IO.h

    @Author: BN
    Copyright 2019 Velleman nv
*/
#ifndef _IO_H_
#define _IO_H_
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "relay.hpp"
#include "input.hpp"
#include "mosfet.hpp"
#include "led.hpp"
#include "channel.hpp"
#define INT_PIN   GPIO_NUM_4
#define INT2_PIN  GPIO_NUM_35

#define ESP_INTR_FLAG_DEFAULT ESP_INTR_FLAG_LOWMED	

#define RELAY_MAX 12
#define INPUT_MAX 13

//void IRAM_ATTR gpio_isr_handler(void* arg);
void Init_IO();
void setMOSFET(Mosfet* mosfet);
//void setLeds(led_t* leds);
void readButton();
Input** readInputs(Input** inputs);
void IO_task(void* arg);
bool convertInputToRelay(Input input);
bool IsExtensionConnected();
//Relay* getRelayFromRelayKey(const char* key);
//void setRelay(uint8_t index,relay_state_t state);
void setLed(Led* led);
bool inputChanged();
void clearInputChanged();
Mosfet* getMosfet(uint8_t index);
void getMosfets(Mosfet* m);
bool isUserInputChanged();
Input** getCurrentInputs();
void initExtPinDirections();
void copyStateRelaysToLeds();
Mosfet* getMosfetById(int id);
Relay* getRelayById(int id);
Led* getLedById(int id);
Relay* getRelays();
void getLeds(Led* leds);
Channel* getChannelById(int id);
void updateIO(void* params);
#endif
