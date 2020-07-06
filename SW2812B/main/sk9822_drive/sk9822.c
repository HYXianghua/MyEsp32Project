
#include <stdio.h>
#include <stdlib.h>

#include "sk9822.h"
#include "driver/spi_master.h"
#include <string.h>

#define PIN_D1_DATA 26
#define PIN_D1_CLK 27

#define PIN_D2_DATA 12
#define PIN_D2_CLK 14

#define NUM_LEDS 40

spi_device_handle_t spi;

void sk9822_drive_init(void)
{
  esp_err_t ret;
  spi_bus_config_t buscfg = {
      .miso_io_num = -1,
      .mosi_io_num = PIN_D1_DATA,
      .sclk_io_num = PIN_D1_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = (NUM_LEDS + 2) * 8 //
  };
  spi_device_interface_config_t devcfg = {
      .clock_speed_hz = 30 * 1000 * 1000, 
      .mode = 0,                          
      .spics_io_num = -1,
      .queue_size = 1,
      .pre_cb = NULL,
  };
  ret = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
  ESP_ERROR_CHECK(ret);
  ret = spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
  ESP_ERROR_CHECK(ret);
}

void sk9822_sand_data(const uint32_t *data, int len)
{
  spi_transaction_t t;
  if (len == 0)
    return;                             //no need to send anything
  memset(&t, 0, sizeof(t));             //Zero out the transaction
  t.length = len * 32;                  //Len is in bytes, transaction length is in bits.
  t.tx_buffer = data;                   //Data
  spi_device_polling_transmit(spi, &t); //Transmit!
  //spi_device_transmit(spi, &t);
}
