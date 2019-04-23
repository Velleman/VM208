/* IO.h

    @Author: BN
    Copyright 2019 Velleman nv
*/
#ifndef _IO_H_
#define _IO_H_
#include <stdbool.h>
#include "TCA6424A.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define INT_PIN   GPIO_NUM_4
#define INT2_PIN  GPIO_NUM_35

#define ESP_INTR_FLAG_DEFAULT ESP_INTR_FLAG_LOWMED	

#define RELAY_MAX 12
#define INPUT_MAX 13
typedef enum {RELAY_OFF=0,RELAY_ON}relay_state_t;
typedef enum {MOSFET_OFF=0,MOSFET_ON}mosfet_state_t;
typedef enum {LED_ON=0,LED_OFF}led_state_t;
typedef enum {INPUT_ON=0,INPUT_OFF}input_state_t;

typedef enum{
  LED_1,
  LED_2,
  LED_3,
  LED_4,
  LED_5,
  LED_6,
  LED_7,
  LED_8,
  LED_9,
  LED_10,
  LED_11,
  LED_12,
}led_index_t;

typedef struct{
  uint8_t pin;
  bool isExtension;
  led_state_t state;
}led_t;

typedef enum {RELAY_1,
              RELAY_2,
              RELAY_3,
              RELAY_4,
              RELAY_5,
              RELAY_6,
              RELAY_7,
              RELAY_8,
              RELAY_9,
              RELAY_10,
              RELAY_11,
              RELAY_12,
            }relay_index_t;
typedef struct{
    uint8_t pin;
    bool isExtension;
    relay_state_t state;
    led_t* led;
}relay_t;

typedef struct{
  uint8_t pin;
  mosfet_state_t state;
}mosfet_t;


typedef enum{
  INPUT_1,
  INPUT_2,
  INPUT_3,
  INPUT_4,
  INPUT_5,
  INPUT_6,
  INPUT_7,
  INPUT_8,
  INPUT_9,
  INPUT_10,
  INPUT_11,
  INPUT_12,
  INPUT_13,
}input_index_t;

typedef struct{
  uint8_t pin;
  bool isExtension;
  input_state_t state;
}input_t;

//void IRAM_ATTR gpio_isr_handler(void* arg);
void Init_IO();
void setRelays(relay_t* relays);
void setMOSFET(mosfet_t* mosfet);
void setLeds(led_t* leds);
void readButton();
input_t* readInputs(input_t* inputs);
void IO_task(void* arg);
relay_state_t convertInputToRelay(input_state_t state);
void getRelays(relay_t* pRelays,uint8_t length);
bool IsExtensionConnected();
relay_t* getRelayFromRelayKey(const char* key);
void setRelay(relay_t* relay);
void setRelay(uint8_t index,relay_state_t state);
void setLed(led_t* led);
bool inputChanged();
void clearInputChanged();
void getCurrentInputState(bool* state);
mosfet_t* getMosfet(uint8_t index);
void getMosfets(mosfet_t* m);
bool isUserInputChanged();
input_t* getCurrentInputs();
void initExtPinDirections();
#endif
