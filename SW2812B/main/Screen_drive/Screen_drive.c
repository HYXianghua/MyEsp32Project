#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "esp_types.h"
#include "driver/timer.h"
#include "driver/gpio.h"

#include "Screen_drive.h"
#include "sk9822_drive/sk9822.h"

#define XXH_DEBUG 0

uint32_t fps_test = 0;
float rotate_speed = 0;                                               //每秒圈数计数
uint32_t lap_time;                                                    //旋转一圈用的时间 0.00001s
static uint8_t timer_evt_state = 1;                                   //播放状态
uint16_t current_angle = 0;                                           //当前角位置
xQueueHandle timer_queue;                                             //用于控制FPS
xQueueHandle frame_queue;                                             //用于控制帧更新
uint32_t screen_display_buff[2][LINE_NUMBER][COUNT_NUMBER + 2] = {0}; //显示数据缓冲区[双缓冲区][每圈线数][每线点数]
uint16_t frame_buff_p = 0;                                            //当前缓冲区
#define screen_updata(S) sk9822_sand_data(S, COUNT_NUMBER + 2);

typedef struct
{
  int type;
} screen_event_t;

enum EnumScreenEvent
{
  ENUM_SCREEN_TIME_IN,    //更新时间到
  ENUM_SCREEN_STOP,       //停止显示
  ENUM_SCREEN_START,      //开始显示
  ENUM_SCREEN_FRAME_RESET //帧重置
};

void vScreenOn()
{
  screen_event_t evt;
  evt.type = ENUM_SCREEN_START;
  xQueueSend(timer_queue, &evt, portMAX_DELAY);
}

void vScreenOff()
{
  screen_event_t evt;
  evt.type = ENUM_SCREEN_STOP;
  xQueueSend(timer_queue, &evt, portMAX_DELAY);
}

void HSVtoRGB(unsigned char *r, unsigned char *g, unsigned char *b, int h, int s, int v)
{

  int i;

  float RGB_min, RGB_max;
  RGB_max = v * 2.55f;
  RGB_min = RGB_max * (100 - s) / 100.0f;

  i = h / 60;
  int difs = h % 60; // factorial part of h

  // RGB adjustment amount by hue
  float RGB_Adj = (RGB_max - RGB_min) * difs / 60.0f;

  switch (i)
  {
  case 0:
    *r = RGB_max;
    *g = RGB_min + RGB_Adj;
    *b = RGB_min;
    break;
  case 1:
    *r = RGB_max - RGB_Adj;
    *g = RGB_max;
    *b = RGB_min;
    break;
  case 2:
    *r = RGB_min;
    *g = RGB_max;
    *b = RGB_min + RGB_Adj;
    break;
  case 3:
    *r = RGB_min;
    *g = RGB_max - RGB_Adj;
    *b = RGB_max;
    break;
  case 4:
    *r = RGB_min + RGB_Adj;
    *g = RGB_min;
    *b = RGB_max;
    break;
  default: // case 5:
    *r = RGB_max;
    *g = RGB_min;
    *b = RGB_max - RGB_Adj;
    break;
  }
}

static void rgb(uint8_t L)
{
  static uint8_t orgb[3] = {0x3c, 0, 0};
  for (uint16_t l = 0; l < LINE_NUMBER; l++)
  {
    HSVtoRGB(&orgb[0], &orgb[1], &orgb[2], l * (360.0 / LINE_NUMBER), 100, 100);
    screen_display_buff[!frame_buff_p][l][1] = ((L) | 0B11100000) | ((uint8_t)(orgb[0]) << 8) | ((uint8_t)(orgb[1]) << 16) | ((uint8_t)((orgb[2])) << 24);
    for (uint16_t d = 1; d <= COUNT_NUMBER; d++)
    {
      screen_display_buff[!frame_buff_p][l][d] = screen_display_buff[!frame_buff_p][l][1];
    }
    screen_display_buff[!frame_buff_p][l][0] = 0;
    screen_display_buff[!frame_buff_p][l][COUNT_NUMBER + 1] = UINT32_MAX;
  }
}

static void hd(uint8_t L)
{
  for (uint16_t l = 0; l < LINE_NUMBER; l++)
  {
    uint8_t buff = l * ((float)(UINT8_MAX) / LINE_NUMBER);

    if (l > (LINE_NUMBER - 10))
    {
      screen_display_buff[!frame_buff_p][l][1] = ((L) | 0B11100000) | (buff << 24);
    }
    else
    {
      screen_display_buff[!frame_buff_p][l][1] = ((L) | 0B11100000) | (buff << 8) | (buff << 16) | (buff << 24);
    }

    for (uint16_t d = 1; d <= COUNT_NUMBER; d++)
    {
      screen_display_buff[!frame_buff_p][l][d] = screen_display_buff[!frame_buff_p][l][1];
    }
    screen_display_buff[!frame_buff_p][l][0] = 0;
    screen_display_buff[!frame_buff_p][l][COUNT_NUMBER + 1] = UINT32_MAX;
  }
}

//清空帧
static void vScreenShutDownDisplay(void)
{
  //printf("清空帧\n");
  for (uint16_t q = 0; q < LINE_NUMBER; q++)
  {
    for (uint16_t l = 1; l <= COUNT_NUMBER; l++)
    {
      screen_display_buff[1][q][l] = 0b111 << 29;
      screen_display_buff[0][q][l] = 0b111 << 29;
    }
    screen_display_buff[1][q][0] = 0;
    screen_display_buff[1][q][COUNT_NUMBER + 1] = UINT32_MAX;
    screen_display_buff[0][q][0] = 0;
    screen_display_buff[0][q][COUNT_NUMBER + 1] = UINT32_MAX;
  }
}

//帧数据更新
static void vFrameUpData(void *arg)
{
  uint8_t evt;
  uint8_t frame_updata_state = 1;
  for (;;)
  {
    xQueueReceive(frame_queue, &evt, portMAX_DELAY); //读取指令
    if (evt == ENUM_FRAME_UPDATA)
    {
      if (frame_updata_state)
      {
        hd(1);
        /* code */
      }
    }
    else if (evt == ENUM_FRAME_PAUSE)
    {
      vScreenShutDownDisplay();
      printf("暂停播放\n");
      frame_updata_state = 0;
    }
    else if (evt == ENUM_FRAME_STARE)
    {
      printf("开始播放\n");
      frame_updata_state = 1;
      /* code */
    }
  }
}

//定时器回调
void IRAM_ATTR vScreenEvtTimerIsrHandler(void *para)
{
  int timer_idx = (int)para;
  uint32_t intr_status = TIMERG0.int_st_timers.val;
  TIMERG0.hw_timer[timer_idx].update = 1;
  if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) //线位置定时器
  {
    fps_test++;
    screen_updata((uint32_t *)(screen_display_buff[frame_buff_p][current_angle])); //更新数据
    if (timer_evt_state)
    {
      //准备播放下一角度数据
      if (++current_angle == LINE_NUMBER) //防止溢出.
      {
        current_angle = 0;
      }
    }

    screen_event_t evt;
    BaseType_t err = xQueueReceiveFromISR(timer_queue, &evt, 0); //读取指令
    if (err == pdTRUE)
    {
      if (evt.type == ENUM_SCREEN_STOP) //关闭播放
      {
        screen_event_t evt;
        evt.type = ENUM_FRAME_PAUSE;
        xQueueSendFromISR(frame_queue, &evt, NULL);
        timer_evt_state = 0;
        current_angle = 0; //重置帧位置
      }
      else if (evt.type == ENUM_SCREEN_START) //开始播放
      {
        screen_event_t evt;
        evt.type = ENUM_FRAME_STARE;
        xQueueSendFromISR(frame_queue, &evt, NULL);
        timer_evt_state = 1;
        current_angle = 0; //重置帧位置
      }
    }
    TIMERG0.int_clr_timers.t0 = 1;
  }
  TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;
}

void IRAM_ATTR vScreenLapTimerIsrHandler(void *para)
{
  int timer_idx = (int)para;
  uint32_t intr_status = TIMERG0.int_st_timers.val;
  TIMERG0.hw_timer[timer_idx].update = 1;
  if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_1) //计算转速用
  {
    lap_time++;
    TIMERG0.int_clr_timers.t1 = 1;
  }
  TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;
}

static void vScreenEvtTimerInit()
{
  timer_config_t config;
  config.divider = TIMER_DIVIDER;
  config.counter_dir = TIMER_COUNT_UP;
  config.counter_en = TIMER_PAUSE;
  config.alarm_en = TIMER_ALARM_EN;
  config.intr_type = TIMER_INTR_LEVEL;
  config.auto_reload = true;

  timer_init(TIMER_GROUP_0, TIMER_0, &config);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x0ULL);
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_SCALE / (LINE_NUMBER * 24));
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_register(TIMER_GROUP_0, TIMER_0, vScreenEvtTimerIsrHandler,
                     (void *)TIMER_0, ESP_INTR_FLAG_IRAM, NULL);
  timer_start(TIMER_GROUP_0, TIMER_0);

  timer_init(TIMER_GROUP_0, TIMER_1, &config);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0x0ULL);
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, (0.00001 * TIMER_SCALE));
  timer_enable_intr(TIMER_GROUP_0, TIMER_1);
  timer_isr_register(TIMER_GROUP_0, TIMER_1, vScreenLapTimerIsrHandler,
                     (void *)TIMER_1, ESP_INTR_FLAG_IRAM, NULL);
  timer_start(TIMER_GROUP_0, TIMER_1);
}

//转速传感IO中断回调
static void IRAM_ATTR vFrameIsrHandler(void *arg)
{
  if ((uint32_t)arg == FRAME_SIGNAL_PIN)
  {
    uint32_t nowlap = lap_time;
    lap_time = 0;
    rotate_speed *= 0.3;
    rotate_speed += 70000.0 / nowlap; //记录转速

    //timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, (TIMER_SCALE) / (LINE_NUMBER * rotate_speed));
    current_angle = 0; //重置帧位置
    frame_buff_p = !frame_buff_p;
    uint8_t buff = ENUM_FRAME_UPDATA;
    xQueueSendFromISR(frame_queue, &buff, 0);
  }
}

//转速传感IO模块
static void vSpeedSensorInit(void)
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
  io_conf.pin_bit_mask = 1ULL << FRAME_SIGNAL_PIN;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = 1;
  gpio_config(&io_conf);
  gpio_set_intr_type(FRAME_SIGNAL_PIN, GPIO_INTR_POSEDGE);
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(FRAME_SIGNAL_PIN, vFrameIsrHandler, (void *)FRAME_SIGNAL_PIN);
}
#if XXH_DEBUG
TimerHandle_t fps_test_timer_handle = NULL;
static void vFpsTestCb(TimerHandle_t xTimer)
{
  screen_event_t evt;
  evt.type = ENUM_SCREEN_FRAME_RESET;
  xQueueSend(timer_queue, &evt, 1);
}
#endif

void vScreenInit(void)
{
  //pvPortMalloc(sizeof(uint32_t) * LINE_NUMBER * (COUNT_NUMBER + 2) * 2);

  timer_queue = xQueueCreate(20, sizeof(screen_event_t));
  frame_queue = xQueueCreate(4, sizeof(uint8_t));
  //初始化显示驱动
  sk9822_drive_init();
  //初始化帧更新任务
  xTaskCreatePinnedToCore(vFrameUpData, "vFrameUpData", 2048, NULL, 4, NULL, 0);
  //启动定时器
  vScreenEvtTimerInit();

#if XXH_DEBUG
  //模拟转速传感器
  fps_test_timer_handle = xTimerCreate(NULL, (100), pdTRUE, NULL, vFpsTestCb);
  xTimerStart(fps_test_timer_handle, 0);
#else
  //初始化转速传感IO
  vSpeedSensorInit();
#endif
}

void vUpSpeed(void)
{
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, (TIMER_SCALE) / (LINE_NUMBER * rotate_speed));
}