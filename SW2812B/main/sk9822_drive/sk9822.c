
#include <stdio.h>
#include <stdlib.h>

#include "sk9822.h"
#include "driver/spi_master.h"
#include <string.h>

// #define PIN_D2_DATA 33
// #define PIN_D2_CLK 25

// #define PIN_D1_DATA 26
// #define PIN_D1_CLK 27

#define PIN_D2_DATA 26
#define PIN_D2_CLK 27

#define PIN_D1_DATA 33
#define PIN_D1_CLK 25

#define SPI_CLOCK_SPEED 15 * 1000 * 1000

#define NUM_LEDS 48

spi_device_handle_t spi1, spi2; //通道一
spi_transaction_t t1, t2;

void vDriveSk9822Init(void)
{
  esp_err_t ret;
  //初始化一号通道
  spi_device_interface_config_t devcfg1 = {
      .clock_speed_hz = SPI_CLOCK_SPEED,
      .mode = 0,
      .spics_io_num = -1,
      .queue_size = 2,
      .pre_cb = NULL,
  };
  spi_bus_config_t buscfg1 = {
      .miso_io_num = -1,
      .mosi_io_num = PIN_D1_DATA,
      .sclk_io_num = PIN_D1_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = (NUM_LEDS + 2) * 8 //
  };
  ret = spi_bus_initialize(HSPI_HOST, &buscfg1, 1);
  ESP_ERROR_CHECK(ret);
  ret = spi_bus_add_device(HSPI_HOST, &devcfg1, &spi1);
  ESP_ERROR_CHECK(ret);

  //初始化二号通道
  spi_device_interface_config_t devcfg2 = {
      .clock_speed_hz = SPI_CLOCK_SPEED,
      .mode = 0,
      .spics_io_num = -1,
      .queue_size = 2,
      .pre_cb = NULL,
  };

  spi_bus_config_t buscfg2 = {
      .miso_io_num = -1,
      .mosi_io_num = PIN_D2_DATA,
      .sclk_io_num = PIN_D2_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = (NUM_LEDS + 2) * 8 //
  };
  ret = spi_bus_initialize(VSPI_HOST, &buscfg2, 2);
  ESP_ERROR_CHECK(ret);
  ret = spi_bus_add_device(VSPI_HOST, &devcfg2, &spi2);
  ESP_ERROR_CHECK(ret);

  memset(&t1, 0, sizeof(t1));
  t1.length = ((NUM_LEDS / 2 + 1) * 32);
  memset(&t2, 0, sizeof(t2));
  t2.length = ((NUM_LEDS / 2 + 1) * 32);
}

void IRAM_ATTR sk9822_sand_data(const uint32_t *pData)
{
  t1.tx_buffer = pData;
  t2.tx_buffer = pData + (NUM_LEDS / 2 + 1);
  spi_device_queue_trans(spi1, &t1, portMAX_DELAY);
  spi_device_queue_trans(spi2, &t2, portMAX_DELAY);
}

void sk9822_sand_data_len(const uint32_t *data, int dev, int len)
{
  spi_transaction_t tlen = {0};
  tlen.length = len * 32;
  tlen.tx_buffer = data;
  // spi_device_queue_trans(spi1, &tlen, portMAX_DELAY);
  // spi_device_queue_trans(spi2, &tlen, portMAX_DELAY);

  if (dev == 1)
  {
    spi_device_polling_transmit(spi1, &tlen);
  }
  else
  {
    spi_device_polling_transmit(spi2, &tlen);
  }
}
