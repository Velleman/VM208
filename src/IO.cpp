/* IO.c

@Author: BN
Copyright 2019 Velleman nv
*/
#include "IO.hpp"
#include <I2CDev.h>
#include "tca_thread_safe.hpp"
#include "esp_log.h"
#include "global.hpp"
xQueueHandle int_evt_queue = NULL;

Channel channels[14];
Relay relays[12];
Mosfet mosfets[2];
Led leds[12];
bool previousInputs[13];
Input *currentInputs[13];
bool _inputChanged = false;
bool _userInputChanged = false;
static const char *TAG = "IO";
bool previousExtConnectedState = false;
TCA6424A_TS tca;
TCA6424A_TS tca_ext(TCA6424A_ADDRESS_ADDR_HIGH);

static void gpio_isr_handler(void *arg)
{
  uint32_t gpio_num = (uint32_t)arg;
  xQueueSendFromISR(int_evt_queue, &gpio_num, NULL);
}

void Init_IO()
{

  Wire.begin(33, 32);

  //init relays,leds and buttons
  for (uint8_t i = 0; i < 4; i++)
  {
    relays[i] = Relay(i + 1, i, false, &tca);
    leds[i] = Led(i + 1, TCA6424A_P14 + i, true, &tca);
    currentInputs[i] = new Input(i + 1, TCA6424A_P10 + i, &tca);

    channels[i] = config.createChannel(i + 1, relays + i, leds + i);
    
  }
  for (int i = 4; i < 12; i++)
  {
    relays[i] = Relay(i + 1, TCA6424A_P00 + (i - 4), false, &tca_ext);
    leds[i] = Led(i + 1, TCA6424A_P20 + (i - 4), true, &tca_ext);
    currentInputs[i] = new Input(i + 1, TCA6424A_P10 + (i - 4), &tca_ext);
    channels[i] = config.createChannel(i + 1, relays + i, leds + i);
  }

  mosfets[0] = Mosfet(1, TCA6424A_P04, false, &tca);
  mosfets[1] = Mosfet(2, TCA6424A_P05, false, &tca);
  //channels[12] = config.createMosfetChannel(13,&mosfets[0]);
  //channels[13] = config.createMosfetChannel(14,&mosfets[1]);
  currentInputs[12] = new Input(13, TCA6424A_P06, &tca);

  //set floating pins as output
  tca.setPinDirection(TCA6424A_P07, TCA6424A_OUTPUT);
  for (size_t i = 0; i < 8; i++)
  {
    tca.setPinDirection(TCA6424A_P20 + i, TCA6424A_OUTPUT);
    tca.writePin(TCA6424A_P20 + i, TCA6424A_LOW);
  }

  pinMode(4, INPUT);

  //Init Interrupt Pin
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  //bit mask of the pins, use GPIO4/35 here
  io_conf.pin_bit_mask = (1ULL << 4);
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-up mode
  //io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  //io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_config(&io_conf);

  gpio_set_direction(GPIO_NUM_35, GPIO_MODE_INPUT);
  gpio_pullup_en(GPIO_NUM_35);
  gpio_set_intr_type(GPIO_NUM_35, GPIO_INTR_NEGEDGE);

  //create a queue to handle gpio event from isr
  int_evt_queue = xQueueCreate(2, sizeof(uint32_t));
  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(INT_PIN, gpio_isr_handler, (void *)INT_PIN);
  if (tca_ext.testConnection())
  {
    ESP_LOGI(TAG, "EXTENSION CONNECTED");
    gpio_isr_handler_add(INT2_PIN, gpio_isr_handler, (void *)INT2_PIN);
  }
}

void initExtPinDirections()
{
  ESP_LOGI(TAG, "initExtPinDirections");
  for (int i = 4; i < 12; i++)
  {
    relays[i].initPin();
    leds[i].initPin();
    currentInputs[i]->initPin();
  }
}

Input **readInputs(Input **inputs)
{
  for (int i = 0; i < INPUT_MAX; i++)
  {
    inputs[i]->read();
  }
  return inputs;
}

void IO_task(void *arg)
{

  uint32_t io_num;
  //bool inputs_state[13];
  for (int i = 0; i < 12; i++)
  {
    bool currentState = currentInputs[i]->read();
    previousInputs[i] = currentState;
    leds[i].setState(currentState);
  }
  xTaskCreate(updateIO,"Update IO",2048,NULL,(tskIDLE_PRIORITY + 2),NULL);
  while (1)
  {

    if (xQueueReceive(int_evt_queue, &io_num, portMAX_DELAY))
    {
      if (io_num == INT2_PIN) //read extension
      {
        for (int i = 4; i < 12; i++)
        {
          bool currentState = currentInputs[i]->read();
          Serial.print(i);
          Serial.print(" ");
          currentState ? Serial.println("True") : Serial.println("False");
          //bool currentState = inputs_state[i];
          if (currentState != previousInputs[i])
          {
            if (currentState == false)
            {
              channels[i].toggle(); //toggle state
              channels[i].clearTimerAndPulse();
              channels[i].disableSheduler();
            }
            previousInputs[i] = currentState;
            _inputChanged = true;
          }
          delay(1);
        }
      }
      else
      {
        for (int i = 0; i < 4; i++)
        {
          bool currentState = currentInputs[i]->read();
          //bool currentState = inputs_state[i];
          if (currentState != previousInputs[i])
          {
            if (currentState == false)
            {
              channels[i].toggle(); //toggle state
              channels[i].clearTimerAndPulse();
              channels[i].disableSheduler();
            }
          }
          previousInputs[i] = currentState;
          _inputChanged = true;
          delay(1);
        }
        bool currentState = currentInputs[12]->read();
        if (currentState != previousInputs[12])
        {
          _userInputChanged = true;
          ESP_LOGI(TAG, "Input has changed");
          previousInputs[12] = currentState;
          _inputChanged = true;
        }
      }
    }
  }
  delay(100);
}

void updateIO(void* params)
{
  uint8_t data;
  for(;;)
  {
    data = 0x00;
    for(int i = 3 ; i != 0;i--)
    {
      
      bool state = leds[i].getState();
      if(state)
      {
        data |= 0x01;//set last bit 1 = LED OFF
      }else{
        data &= 0xFE;//set last bit 0 = LED ON
      }
      data <<=1;
    }
    data<<=4;
    tca.writeByte(TCA6424A_RA_OUTPUT1,data);
  vTaskDelay(100/portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

bool convertInputToRelay(Input input)
{

  if (input.read() == false)
  {
    return true;
  }
  else
  {
    return false;
  }
}

Relay *getRelays()
{
  return relays;
}

void getLeds(Led *pLeds)
{
  pLeds = leds;
}

bool IsExtensionConnected()
{
  bool currentState = tca_ext.testConnection();
  if (currentState == true && previousExtConnectedState == false)
  {
    initExtPinDirections();
    copyStateRelaysToLeds();
    gpio_isr_handler_add(INT2_PIN, gpio_isr_handler, NULL);
  }
  if (previousExtConnectedState == true && currentState == false)
  {
    gpio_isr_handler_remove(INT2_PIN);
  }
  previousExtConnectedState = currentState;
  return currentState;
}

Mosfet *getMosfetById(int id)
{
  return &mosfets[id - 1];
}

Relay *getRelayById(int id)
{
  return &relays[id - 1];
}

Channel *getChannelById(int id)
{
  xSemaphoreTake(g_MutexChannel, portMAX_DELAY);
  if (id <= 12 && id > 0)
  {
    xSemaphoreGive(g_MutexChannel);
    return channels + (id - 1);
  }
  else
  {
    ESP_LOGE("IO", "Channel ID is invalid: id is %i", id);
    xSemaphoreGive(g_MutexChannel);
    return nullptr;
  }
}

Led *getLedById(int id)
{
  return &leds[id - 1];
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

bool isUserInputChanged()
{
  return _userInputChanged;
}

Input **getCurrentInputs()
{
  return currentInputs;
}

void copyStateRelaysToLeds()
{
  for (int i = 0; i < 12; i++)
  {
    channels[i].getState() ? channels[i].turnOn() : channels[i].turnOff();
  }
}
