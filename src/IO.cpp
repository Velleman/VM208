/* IO.c

@Author: BN
Copyright 2019 Velleman nv
*/
#include "IO.h"
#include <I2CDev.h>
#include <TCA6424A.h>
#include "esp_log.h"
xQueueHandle int_evt_queue = NULL;

relay_t relays[12];
mosfet_t mosfets[2];
led_t leds[12];
input_t previousInputs[13];
input_t currentInputs[13];
bool _inputChanged=false;
bool _userInputChanged = false;
static const char *TAG = "IO";
bool previousExtConnectedState = false;
TCA6424A tca;
TCA6424A tca_ext;

static void gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(int_evt_queue, &gpio_num, NULL);
}

void Init_IO()
{
  //TODO: init TCA and interrupt
    //tca.initialize();
  Wire.begin(33,32);

  //specify ADDRESS HIGH
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
    currentInputs[i].pin = (i+TCA6424A_P10) - 4;
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

  //Init Interrupt Pin
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  //bit mask of the pins, use GPIO4/35 here
  io_conf.pin_bit_mask = (1ULL<<4);
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-up mode
  //io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  //io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_config(&io_conf);

  gpio_set_direction(GPIO_NUM_35,GPIO_MODE_INPUT);
  gpio_set_intr_type(GPIO_NUM_35,GPIO_INTR_NEGEDGE);

  //create a queue to handle gpio event from isr
  int_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(INT_PIN, gpio_isr_handler, (void*) INT_PIN);
  
      ESP_LOGI(TAG, "isr PIN 2 ");
      gpio_isr_handler_add(INT2_PIN, gpio_isr_handler, (void*) INT2_PIN);
  
}

void initExtPinDirections()
{
  for(int i = 4; i<12;i++)
  {
    tca_ext.setPinDirection(relays[i].pin,TCA6424A_OUTPUT);
    tca_ext.setPinDirection(currentInputs[i].pin,TCA6424A_INPUT);
    tca_ext.setPinDirection(leds[i].pin,TCA6424A_OUTPUT);
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
  m = mosfets;
}

void readButton()
{

}

input_t* readInputs(input_t* inputs){
  for(int i=0;i<INPUT_MAX;i++)
  {
    if((inputs+i)->isExtension)
    {
      (inputs+i)->state = tca_ext.readPin((inputs+i)->pin)?INPUT_OFF:INPUT_ON;
    }
    else{
      (inputs+i)->state = tca.readPin((inputs+i)->pin)?INPUT_OFF:INPUT_ON;
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
    if(xQueueReceive(int_evt_queue, &io_num, portMAX_DELAY)) {
      ESP_LOGI(TAG, "interrupt");
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
    }
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
  bool currentState = tca_ext.testConnection();
  if(currentState == true && previousExtConnectedState == false)
  {
    initExtPinDirections();
    relay_t* r;
    getRelays(r,12);
    setRelays(r);
    gpio_isr_handler_add(INT2_PIN,gpio_isr_handler,NULL);
  }
  if(previousExtConnectedState == true && currentState == false)
  {
     gpio_isr_handler_remove(INT2_PIN);
  }
  previousExtConnectedState = currentState;
  return currentState;
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

input_t* getCurrentInputs()
{
  return currentInputs;
}
