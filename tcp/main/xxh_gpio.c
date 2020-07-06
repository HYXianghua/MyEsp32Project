#include "driver/gpio.h"
#include "xxh_gpio.h"

void vMyGpioInit(uint64_t pins)
{
  gpio_config_t io_conf;
  // disable interrupt
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  // set as output mode
  io_conf.mode = GPIO_MODE_OUTPUT;
  // bit mask of the pins that you want to set,e.g.GPIO18/19
  io_conf.pin_bit_mask = pins;
  // disable pull-down mode
  io_conf.pull_down_en = 0;
  // disable pull-up mode
  io_conf.pull_up_en = 0;
  // configure GPIO with the given settings
  gpio_config(&io_conf);
  // addTask(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);
  /*
  //interrupt of rising edge
  io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
  //bit mask of the pins, use GPIO4/5 here
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-up mode
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);

  //change gpio intrrupt type for one pin
  gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

  //create a queue to handle gpio event from isr
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

  //start gpio task
  xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void
  *)GPIO_INPUT_IO_0);
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void
  *)GPIO_INPUT_IO_1);

  //remove isr handler for gpio number.
  gpio_isr_handler_remove(GPIO_INPUT_IO_0);
  //hook isr handler for specific gpio pin again
  gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void
  *)GPIO_INPUT_IO_0);*/
}

uint64_t vMygpioInidPins(uint8_t s[])
{
  static uint64_t pins = 0;
  uint8_t a = 0;
  while (s[a])
  {
    pins |= (1ULL << s[a]);
    a++;
  }
  vMyGpioInit(pins);
  return pins;
}

void vMyGpioControl(int pin, int x) { gpio_set_level(pin, x); }
