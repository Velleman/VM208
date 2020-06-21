/* IO.c

@Author: BN
Copyright 2019 Velleman nv
*/
#include "IO.hpp"
#include <I2CDev.h>
#include "tca_thread_safe.hpp"
#include "esp_log.h"
#include "mail.hpp"
#include "ETH.h"

xQueueHandle int_evt_queue = NULL;
unsigned long previousTime = 0;

static void gpio_isr_handler(void *arg)
{
  if ((millis() - previousTime) >  200)
  {
    previousTime = millis();
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(int_evt_queue, &gpio_num, NULL);
  }
}

void Init_IO(bool setState)
{

  Wire.begin(33, 32, 25000);
  mm.DetectModules();
  //vm208.initialize();
  //vm208ex.initialize();

  /*gpio_pullup_en(GPIO_NUM_33);
  gpio_pullup_en(GPIO_NUM_32);
  gpio_pulldown_dis(GPIO_NUM_33);
  gpio_pulldown_dis(GPIO_NUM_32);*/

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
  //gpio_isr_handler_add(INT_PIN, gpio_isr_handler, (void *)INT_PIN);

  if (mm.getAmount()) //if a module is detected the second interrupt pin is used.
    //gpio_isr_handler_add(INT2_PIN, gpio_isr_handler, (void *)INT2_PIN);

  Serial.println("Found modules: ");
  Serial.println(mm.getAmount());
  /*for(int i = 0;i<4;i++)
  {
    
  }
  delay(1000);
  for (int i = 0; i < mm.getAmountOfModulesOnInterface(0); i++)
  {
      mm.getModuleFromInterface(0,i)->turnAllChannelsOn();
  }
  delay(1000);
  for (int i = 0; i < mm.getAmountOfModulesOnInterface(0); i++)
  {
      mm.getModuleFromInterface(0,i)->turnAllChannelsOff();
  }
  delay(1000);
  for (int i = 0; i < mm.getAmountOfModulesOnInterface(0); i++)
  {
      mm.getModuleFromInterface(0,i)->turnAllChannelsOn();
  }
  delay(1000);
  for (int i = 0; i < mm.getAmountOfModulesOnInterface(0); i++)
  {
      mm.getModuleFromInterface(0,i)->turnAllChannelsOn();
  }*/
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
  //xTaskCreate(updateIO, "checkExtension", 4096, NULL, (tskIDLE_PRIORITY + 2), NULL);
  while (1)
  {

    if (xQueueReceive(int_evt_queue, &io_num, portMAX_DELAY) )//|| digitalRead(INT_PIN) == LOW)
    {
      Serial.println("Handle Interrupt");
      if (io_num == INT2_PIN) //read extension
      {
        toggleChannel((VM208EX *)mm.getModule(1), mm.getModule(1)->getPressedButton());
      }
      else
      {
        toggleChannel(mm.getBaseModule(), mm.getBaseModule()->getPressedButton());
      }
    }
    io_num = 0;
  }
  delay(200);
}

void toggleChannel(VM208 *ex, uint8_t channel)
{
  if (channel && channel < 5)
  {
    VM208 vm = *ex;
    vm[channel - 1].toggle();
  }
}

void toggleChannel(VM208EX *ex, uint8_t channel)
{
  if (channel)
  {
    (*ex)[channel-1].toggle();
    //(*(ex))[channel - 1].toggle();
  }
}

RelayChannel *getRelayChannelById(int id)
{
  if (id <= 4)
  {
    VM208 *vm208 = (VM208 *)mm.getModule(0);
    return &(*vm208)[id - 1];
  }
  else
  {
    VM208EX *ex = (VM208EX *)mm.getModule(id / 8);
    return &(*ex)[id % 8];
  }
}