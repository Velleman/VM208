/* IO.c

@Author: BN
Copyright 2019 Velleman nv
*/
#include "IO.h"
#include <I2CDev.h>
#include <TCA6424A.h>

//xQueueHandle int_evt_queue = NULL;

relay_t relays[12];
mosfet_t mosfets[2];
led_t leds[12];
input_t previousInputs[13];
input_t currentInputs[13];
bool _inputChanged=false;
bool _userInputChanged = false;
//static const char *TAG = "IO";

TCA6424A tca;
TCA6424A tca_ext;

void IRAM_ATTR isr() {
    //Serial.println("INTERRUPT TRIGGERED");
}

void Init_IO()
{
  //TODO: init TCA and interrupt
    //tca.initialize();
  Wire.begin(33,32);
  tca.setPinDirection(TCA6424A_P00,TCA6424A_OUTPUT);
  tca.writePin(TCA6424A_P00,TCA6424A_HIGH);
  
  tca_ext = TCA6424A(TCA6424A_ADDRESS_ADDR_HIGH);

  //init data leds
  for(int i=0;i<4;i++)
  {
    leds[i].pin = i+12;
    leds[i].isExtension = false;
    leds[i].state = LED_OFF;
  }
  for(int i=4;i<12;i++)
  {
    leds[i].pin = i+TCA6424A_P20 -4;
    leds[i].isExtension = true;
    leds[i].state = LED_OFF;
  }
  /******************************************************************************/
  //Init data relays
  for(int i =0;i<4;i++)
  {

    relays[i].pin = i;
    relays[i].isExtension = false;
    relays[i].state = RELAY_OFF;
    relays[i].led = leds+i;
  }
  for(int i =4;i<12;i++)
  {
    relays[i].pin = i - 4;
    relays[i].isExtension = true;
    relays[i].state = RELAY_OFF;
    relays[i].led = leds+i;
  }
  /******************************************************************************/
  //init data inputs

  for(int i=0;i<4;i++)
  {
    currentInputs[i].pin = i+TCA6424A_P10;
    currentInputs[i].isExtension = false;
    currentInputs[i].state = INPUT_OFF;
  }

  for(int i=4;i<12;i++)
  {
    currentInputs[i].pin = i+TCA6424A_P10 - 4;
    currentInputs[i].isExtension = true;
    currentInputs[i].state = INPUT_OFF;
  }

  currentInputs[12].pin = TCA6424A_P06;
  currentInputs[12].isExtension = false;
  currentInputs[12].state = INPUT_OFF;

  mosfets[0].pin = TCA6424A_P04;
  mosfets[1].pin = TCA6424A_P05;
  mosfets[0].state = MOSFET_OFF;
  mosfets[1].state = MOSFET_OFF;
  /******************************************************************************/

  //RELAYS
  for(int i=0;i<RELAY_MAX;i++)
  {
    if(relays[i].isExtension)
    {
      tca_ext.setPinDirection(relays[i].pin,TCA6424A_OUTPUT);
    }
    else
    {
      tca.setPinDirection(relays[i].pin,TCA6424A_OUTPUT);
    }

  }

  //MOSFETS
  tca.setPinDirection(mosfets[0].pin,TCA6424A_OUTPUT);
  tca.setPinDirection(mosfets[1].pin,TCA6424A_OUTPUT);

  //inputs
  for(int i=0;i<INPUT_MAX;i++)
  {
    if(currentInputs[i].isExtension)
    {
      tca_ext.setPinDirection(currentInputs[i].pin,TCA6424A_INPUT);
    }
    else
    {
      tca.setPinDirection(currentInputs[i].pin,TCA6424A_INPUT);
    }
  }
  //leds
  for(int i=0;i<RELAY_MAX;i++)
  {
    if(leds[i].isExtension)
    {
      tca_ext.setPinDirection(leds[i].pin,TCA6424A_OUTPUT);
    }
    else
    {
      tca.setPinDirection(leds[i].pin,TCA6424A_OUTPUT);
    }
  }

  //set floating pins as output
  tca.setPinDirection(TCA6424A_P07,TCA6424A_OUTPUT);
  for (size_t i = 0; i < 8; i++) {
    tca.setPinDirection(TCA6424A_P20+i,TCA6424A_OUTPUT);
    tca.writePin(TCA6424A_P20+i,TCA6424A_HIGH);
  }
  setRelays(relays);
  setLeds(leds);
  setMOSFET(&mosfets[0]);
  setMOSFET(&mosfets[1]);
  pinMode(4,INPUT);
    attachInterrupt(4, isr, FALLING);
  bool connected = IsExtensionConnected();
  if(connected)
  {
      pinMode(35,INPUT);
        attachInterrupt(35, isr, FALLING);
    
  }
}

void setRelays(relay_t* ptrRelays)
{
  for(int i=0;i<RELAY_MAX;i++)
  {
    if((ptrRelays+i)->isExtension)
    {
      tca_ext.writePin((ptrRelays+i)->pin,(ptrRelays+i)->state);
    }
    else{
      tca.writePin((ptrRelays+i)->pin,(ptrRelays+i)->state);
    }

  }
}

void setRelay(relay_t* relay)
{
  if(relay->isExtension)
  {
    tca_ext.writePin(relay->pin,relay->state);
  }
  else{
    tca.writePin(relay->pin,relay->state);
  }
  relay->led->state = (relay->state == RELAY_ON)? LED_ON : LED_OFF;
  setLed(relay->led);
}

void setRelay(uint8_t index,relay_state_t state)
{
  relays[index].state = state;
  setRelay(relays + index);
}

void setLed(led_t* led)
{
  if(led->isExtension)
  {
    tca_ext.writePin(led->pin,led->state);
  }
  else{
    tca.writePin(led->pin,led->state);
  }
}

void setMOSFET(mosfet_t* mosfet)
{
  tca.writePin(mosfet->pin,mosfet->state);
}

void setLeds(led_t* ptrLeds)
{
  for(int i=0;i<RELAY_MAX;i++)
  {
    if((ptrLeds+i)->isExtension)
    {
      tca_ext.writePin((ptrLeds+i)->pin,(ptrLeds+i)->state);
    }
    else{
      tca.writePin((ptrLeds+i)->pin,(ptrLeds+i)->state);
    }
    //printf("set leds: %d Index:%d\r\n",(ptrLeds+i)->pin,i);
  }
}

mosfet_t* getMosfet(uint8_t index)
{
  return mosfets + index;
}

void getMosfets(mosfet_t* m)
{
  mosfet_t* mbuf = m;
  mbuf = mosfets;
}

void readButton()
{

}

input_t* readInputs(input_t* inputs){
  for(int i=0;i<INPUT_MAX;i++)
  {
    if((inputs+i)->isExtension)
    {
      (inputs+i)->state = tca_ext.readPin((inputs+i)->pin)?INPUT_ON:INPUT_OFF;
    }
    else{
      (inputs+i)->state = tca.readPin((inputs+i)->pin)?INPUT_ON:INPUT_OFF;
    }
    //printf("read input: %d\r\n",(inputs+i)->state);
  }
  return inputs;
}

void IO_task(void* arg)
{
  uint32_t io_num;
  readInputs(currentInputs);
  for(int i=0;i<INPUT_MAX-1;i++)
  {
    previousInputs[i] = currentInputs[i];
    relays[i].state = convertInputToRelay(currentInputs[i].state);
    leds[i].state = (led_state_t)currentInputs[i].state;
  }
  setRelays(relays);
  setLeds(leds);
  while (1) {
    //if(xQueueReceive(int_evt_queue, &io_num, portMAX_DELAY)) {
      //printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
      readInputs(currentInputs);
      for(int i=0;i<INPUT_MAX;i++)
      {
        if(currentInputs[i].state != previousInputs[i].state)
        {
          if(i<12)
          {
            relays[i].state = convertInputToRelay(currentInputs[i].state);
            leds[i].state = (led_state_t)currentInputs[i].state;
          }
          else{
            _userInputChanged = true;
          }
          previousInputs[i].state = currentInputs[i].state;
          _inputChanged = true;
        }
      }
      setRelays(relays);
      setLeds(leds);
    
    vTaskDelay(100/portTICK_RATE_MS);
  }
}

relay_state_t convertInputToRelay(input_state_t state)
{
    if(state == INPUT_OFF)
    {
      return RELAY_OFF;
    }
    else
    {
      return RELAY_ON;
    }
}

void getRelays(relay_t* pRelays,uint8_t length)
{
  for(int i=0;i<length;i++)
  {
    *(pRelays+i) = relays[i];
  }
}

bool IsExtensionConnected()
{
    return false;
}

relay_t* getRelayFromRelayKey(const char* key)
{
    //asume relay is the key
    int index = atoi(key+5);
    ESP_LOGI(TAG,"%d",index);
    return &relays[index-1];
}

bool inputChanged()
{
  return _inputChanged;
}
void clearInputChanged()
{
  _inputChanged = false;
  _userInputChanged = false;
}

void getCurrentInputState(bool* state)
{
  *state = currentInputs[12].state;
}

bool isUserInputChanged()
{
  return _userInputChanged;
}
