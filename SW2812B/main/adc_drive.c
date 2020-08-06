#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC1_GPIO35_CHANNEL; //GPIO35 if ADC1
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

void vAdcInit(void)
{
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(channel, atten);
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}

float getCellVoltage(void)
{
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw((adc1_channel_t)channel), adc_chars);
  return voltage / 510.869f;
}
