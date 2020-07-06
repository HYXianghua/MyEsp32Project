#ifndef SCREEN_DRIVE_H
#define SCREEN_DRIVE_H
#include <stdint.h>

#define TIMER_DIVIDER 8                              //预分频
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) //实际频率
#define FPS 10                                       //每秒更新频率
#define LINE_NUMBER 200                              //单圈细分数
#define COUNT_NUMBER 40                              //LED的数量

#define FRAME_SIGNAL_PIN 34
#define GPIO_INPUT_PIN_SEL (1ULL << FRAME_SIGNAL_PIN)
#define ESP_INTR_FLAG_DEFAULT 0

extern uint32_t screen_display_buff[2][LINE_NUMBER][COUNT_NUMBER + 2]; //显示数据缓冲区

enum frame_updata_enum
{
  ENUM_FRAME_UPDATA, //更新时间到
  ENUM_FRAME_PAUSE,  //暂停
  ENUM_FRAME_STARE   //开始
};

void vScreenOn(void);
void vScreenOff(void);
void vScreenInit(void);
void vUpSpeed(void);
#endif