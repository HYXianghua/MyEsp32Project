#ifndef WS2812_CONTROL_H
#define WS2812_CONTROL_H
#include <stdint.h>

#ifndef NUM_LEDS
#define NUM_LEDS 13
#endif

#define LED_RMT_TX_CHANNEL RMT_CHANNEL_0
#define LED_RMT_TX_GPIO 33

#define BITS_PER_LED_CMD 24
#define LED_BUFFER_ITEMS ((NUM_LEDS * BITS_PER_LED_CMD))

#define T0H 16 // bit 0 high level time
#define T1H 34 // bit 1 high level time
#define TL 50  // one bit time

// 初始化相关设备
void ws2812_drive_init(void);
struct led_state
{
  uint32_t leds[NUM_LEDS];
};

//更新led到新的状态。根据需要调用。
//此函数将阻塞当前任务，直到RMT外围设备完成发送
//整个序列。
void ws2812_write_leds(uint32_t *data);

#endif