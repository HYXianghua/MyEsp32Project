#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC1_GPIO35_CHANNEL; //GPIO35 if ADC1
static const adc_atten_t atten = ADC_ATTEN_DB_11;

void app_main()
{
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(channel, atten);
  //Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));

  //Continuously sample ADC1
  while (1)
  {
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++)
    {
      adc_reading += adc1_get_raw((adc1_channel_t)channel);
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    printf("Raw: %d\tVoltage: %0.2fV\n", adc_reading, voltage / 500.0);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
