#ifndef __SCREEN_DRIVE_H__
#define __SCREEN_DRIVE_H__
#include <stdint.h>

#define TIMER_DIVIDER 8                              //预分频
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) //实际频率
#define FPS 10                                       //每秒更新频率
#define LINE_NUMBER 192                              //单圈细分数
#define COUNT_NUMBER 48                              //LED的数量

#define FRAME_SIGNAL_PIN 16
#define GPIO_INPUT_PIN_SEL (1ULL << FRAME_SIGNAL_PIN)
#define ESP_INTR_FLAG_DEFAULT 0

typedef uint32_t frame_type[LINE_NUMBER][COUNT_NUMBER + 2]; //[每圈线数][每线点数]
extern frame_type screen_display_buff[2];                   //显示数据缓冲区[双缓冲区]
extern uint32_t BrightnessMatrix[COUNT_NUMBER + 2];

typedef void (*frameUpDataFunType)(frame_type *); /* 帧更新回调程序 */

// enum
// {

// } framePlayCbEnum;
// typedef void (*framePlayCb)(uint16_t); /* 帧事件回调 */

typedef struct
{
  uint8_t L;
  uint8_t R;
  uint8_t G;
  uint8_t B;
} ScreenLRGBType;

typedef union
{
  uint32_t DATA;
  ScreenLRGBType LRGB;
} ScreenLRGBUnion;

void vScreenOn(void);
void vScreenOff(void);
void vScreenPause(void);
float getRotateSpeed(void);
void vScreenInit(void);
void RegisterPlayer(frameUpDataFunType fun);
void ShowVoid(frame_type *pframe);

#endif