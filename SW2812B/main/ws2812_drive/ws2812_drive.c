#include "ws2812_drive.h"
#include "driver/rmt.h"

// RMT 数据缓冲
rmt_item32_t led_data_buffer[LED_BUFFER_ITEMS];

void ws2812_drive_init(void)
{
  rmt_config_t config;
  config.rmt_mode = RMT_MODE_TX;
  config.channel = LED_RMT_TX_CHANNEL;
  config.gpio_num = LED_RMT_TX_GPIO;
  config.mem_block_num = 3;
  config.tx_config.loop_en = false;
  config.tx_config.carrier_en = false;
  config.tx_config.idle_output_en = true;
  config.tx_config.idle_level = 0;
  config.clk_div = 2;
  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

static void setup_rmt_data_buffer(uint32_t *data)
{
  for (uint32_t led = 0; led < NUM_LEDS; led++)
  {
    uint32_t bits_to_send = data[led];
    uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);
    for (uint32_t bit = 0; bit < BITS_PER_LED_CMD; bit++)
    {
      uint32_t bit_is_set = bits_to_send & mask;
      led_data_buffer[led * BITS_PER_LED_CMD + bit] = bit_is_set ? (rmt_item32_t){{{T1H, 1, TL - T1H, 0}}} : (rmt_item32_t){{{T0H, 1, TL - T0H, 0}}};
      mask >>= 1;
    }
  }
}

void ws2812_write_leds(uint32_t *data)
{
  setup_rmt_data_buffer(data);
  ESP_ERROR_CHECK(rmt_wait_tx_done(LED_RMT_TX_CHANNEL, portMAX_DELAY));
  ESP_ERROR_CHECK(rmt_write_items(LED_RMT_TX_CHANNEL, led_data_buffer, LED_BUFFER_ITEMS, false));
}
