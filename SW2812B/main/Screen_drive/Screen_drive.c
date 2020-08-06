#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/xtensa_api.h"

#include "esp_log.h"
#include "esp_types.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"

#include "user_main.h"

static const char *TAG = "[SCREEN]";

#define XXH_DEBUG 0
#define LAP_TIMER_INIT 1

//显示函数
#define screen_updata sk9822_sand_data

#define LAP_TIMER TIMER_GROUP_0, TIMER_1   //计时定时器
#define FRAME_TIMER TIMER_GROUP_0, TIMER_0 //中断计时器

DMA_ATTR frame_type screen_display_buff[2];           //显示数据缓冲区[双缓冲区]
DMA_ATTR uint32_t BrightnessMatrix[COUNT_NUMBER + 2]; //亮度矩阵
static uint16_t frameBufferAddr;                      //当前缓冲区地址

static frame_type(*pNowDisplayBuff);  //当前帧缓冲区地址指针
static frame_type(*pNextDisplayBuff); //下一帧缓冲区地址指针

frameUpDataFunType frameUpDataApp;

static float rotateSpeed; //每秒圈数计数
static uint64_t timer0ValueBuff = (TIMER_SCALE / (LINE_NUMBER * 24));

static uint32_t currentAngle; //当前角位置
xQueueHandle frame_queue;     //用于控制帧更新

typedef struct
{
  int typ;
  uint64_t lap;
} screen_event_t;

enum EnumScreenEvent
{
  ENUM_SCREEN_TIME_IN,    //更新时间到
  ENUM_SCREEN_STOP,       //停止显示
  ENUM_SCREEN_START,      //开始显示
  ENUM_SCREEN_PAUSE,      //暂停显示
  ENUM_SCREEN_FRAME_RESET //帧重置
};

void vScreenOn(void)
{
  screen_event_t evt;
  evt.typ = ENUM_SCREEN_START;
  xQueueSend(frame_queue, &evt, portMAX_DELAY);
}

void vScreenOff(void)
{
  screen_event_t evt;
  evt.typ = ENUM_SCREEN_STOP;
  xQueueSend(frame_queue, &evt, portMAX_DELAY);
}

void vScreenPause(void)
{
  screen_event_t evt;
  evt.typ = ENUM_SCREEN_PAUSE;
  xQueueSend(frame_queue, &evt, portMAX_DELAY);
}

float getRotateSpeed(void)
{
  return rotateSpeed;
}

/*
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

static void rgb(uint8_t L, frame_type *pDisplayBuff)
{
  static uint8_t orgb[3] = {0x3c, 0, 0};
  for (uint16_t l = 0; l < LINE_NUMBER; l++)
  {
    HSVtoRGB(&orgb[0], &orgb[1], &orgb[2], l * (360.0 / LINE_NUMBER), 100, 100);
    uint32_t lineDataBuff = ((uint8_t)(orgb[0]) << 8) | ((uint8_t)(orgb[1]) << 16) | ((uint8_t)((orgb[2])) << 24);
    for (uint16_t d = 0; d <= COUNT_NUMBER; d++)
    {
      (*pDisplayBuff)[l][d] = ((L + d) | 0B11100000) | lineDataBuff;
    }
    (*pDisplayBuff)[l][0] = 0;
    (*pDisplayBuff)[l][COUNT_NUMBER + 1] = UINT32_MAX;
  }
}
*/

//清空帧
static void
vScreenShutDownDisplay(void)
{
  uint32_t *p = (uint32_t *)(screen_display_buff);
  for (uint16_t q = 0; q < sizeof(screen_display_buff) / 4; q++)
  {
    *p++ = 0b11100000;
  }
  for (uint16_t q = 0; q < LINE_NUMBER; q++)
  {
    (*pNowDisplayBuff)[q][0] = 0;
    //screen_display_buff[1][q][COUNT_NUMBER + 1] = UINT32_MAX;
    (*pNextDisplayBuff)[q][0] = 0;
    //screen_display_buff[0][q][COUNT_NUMBER + 1] = UINT32_MAX;
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
    screen_updata((*pNowDisplayBuff)[currentAngle]); //更新数据
    //准备播放下一角度数据
    if (++currentAngle == LINE_NUMBER) //防止溢出.
    {
      currentAngle = 0;
    }
    TIMERG0.int_clr_timers.t0 = 1;
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

  timer_init(FRAME_TIMER, &config);
  timer_set_counter_value(FRAME_TIMER, 0x0ULL);
  timer_set_alarm_value(FRAME_TIMER, TIMER_SCALE / (LINE_NUMBER * 24));
  timer_enable_intr(FRAME_TIMER);
  timer_isr_register(FRAME_TIMER, vScreenEvtTimerIsrHandler,
                     (void *)TIMER_0, ESP_INTR_FLAG_IRAM, NULL);
  timer_start(FRAME_TIMER);

#if LAP_TIMER_INIT
  config.divider = TIMER_BASE_CLK / 100000;
  config.auto_reload = false;
  timer_init(LAP_TIMER, &config);
  timer_set_counter_value(LAP_TIMER, 0x0ULL);
  timer_set_alarm_value(LAP_TIMER, UINT64_MAX);
  //timer_enable_intr(LAP_TIMER);
  //timer_isr_register(LAP_TIMER, vScreenLapTimerIsrHandler, (void *)TIMER_1, ESP_INTR_FLAG_IRAM, NULL);
  timer_start(LAP_TIMER);
#endif
}

//帧数据更新
static void vFrameUpData(void *arg)
{
  screen_event_t evt;
  static uint8_t frame_updata_state = 1;
  vScreenEvtTimerInit();
  for (;;)
  {
    xQueueReceive(frame_queue, &evt, portMAX_DELAY); //读取指令
    if (evt.typ == ENUM_SCREEN_TIME_IN)
    {
      rotateSpeed *= 0.3;
      rotateSpeed += 70000.0 / evt.lap; //记录转速
      timer0ValueBuff = (TIMER_SCALE / (LINE_NUMBER * rotateSpeed));
      if (frame_updata_state)
      {
        frameUpDataApp(pNextDisplayBuff);
      }
      else
      {
        /* code */
      }
    }
    else if (evt.typ == ENUM_SCREEN_PAUSE)
    {
      ESP_LOGI(TAG, "暂停播放");
      frame_updata_state = 0;
    }
    else if (evt.typ == ENUM_SCREEN_STOP)
    {
      vScreenShutDownDisplay();
      ESP_LOGI(TAG, "停止播放");
      frame_updata_state = 0;
    }
    else if (evt.typ == ENUM_SCREEN_START)
    {
      ESP_LOGI(TAG, "开始播放");
      frame_updata_state = 1;
    }
  }
}

//转速传感IO中断回调
static void IRAM_ATTR vFrameIsrHandler(void *arg)
{
  static uint64_t nowlap;
  if ((uint32_t)arg == FRAME_SIGNAL_PIN)
  {
    static uint8_t upRotateSpeed = 0;
    portMUX_TYPE timer_spinlock = portMUX_INITIALIZER_UNLOCKED;

    currentAngle = 0; //重置帧位置
    frameBufferAddr = !frameBufferAddr;
    pNowDisplayBuff = screen_display_buff + frameBufferAddr;
    pNextDisplayBuff = screen_display_buff + !frameBufferAddr;

    portENTER_CRITICAL_ISR(&timer_spinlock);
    if (upRotateSpeed++ == 24)
    {
      //设置中断间隔
      upRotateSpeed = 0;
      TIMERG0.hw_timer[TIMER_0].alarm_high = (uint32_t)(timer0ValueBuff >> 32);
      TIMERG0.hw_timer[TIMER_0].alarm_low = (uint32_t)timer0ValueBuff;
      //置零计时器
      TIMERG0.hw_timer[TIMER_0].load_high = (uint32_t)(0);
      TIMERG0.hw_timer[TIMER_0].load_low = (uint32_t)0;
      TIMERG0.hw_timer[TIMER_0].reload = 1;
    }
    //读取间隔时间
    TIMERG0.hw_timer[TIMER_1].update = 1;
    nowlap = ((uint64_t)TIMERG0.hw_timer[TIMER_1].cnt_high << 32) | (TIMERG0.hw_timer[TIMER_1].cnt_low);
    //置零计时器
    TIMERG0.hw_timer[TIMER_1].load_high = (uint32_t)(0);
    TIMERG0.hw_timer[TIMER_1].load_low = (uint32_t)(0);
    TIMERG0.hw_timer[TIMER_1].reload = 1;
    portEXIT_CRITICAL_ISR(&timer_spinlock);

    screen_event_t buff = {
        .typ = ENUM_SCREEN_TIME_IN,
        .lap = nowlap};
    xQueueSendFromISR(frame_queue, &buff, 0);
  }
}

//转速传感IO模块
static void vSpeedSensorInit(void)
{
  gpio_config_t io_conf;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
  io_conf.pin_bit_mask = 1ULL << FRAME_SIGNAL_PIN;
  gpio_config(&io_conf);
  //设置中断模式
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(FRAME_SIGNAL_PIN, vFrameIsrHandler, (void *)FRAME_SIGNAL_PIN);
}

#if XXH_DEBUG
TimerHandle_t fps_test_timer_handle = NULL;
static void vFpsTestCb(TimerHandle_t xTimer)
{
  screen_event_t evt;
  evt.type = ENUM_SCREEN_FRAME_RESET;
  xQueueSend(frame_queue, &evt, 1);
}
#endif

void ShowVoid(frame_type *pframe)
{
  uint32_t *p = (uint32_t *)(pframe);
  for (uint16_t q = 0; q < sizeof(pframe) / 4; q++)
  {
    *p++ = 0b11100000;
  }
  for (uint16_t q = 0; q < LINE_NUMBER; q++)
  {
    (*pframe)[q][0] = 0;
    (*pframe)[q][25] = 0;
  }
};

void vScreenInit(void)
{
  pNextDisplayBuff = screen_display_buff + 1;
  pNowDisplayBuff = screen_display_buff + 0;
  ScreenLRGBUnion BUFF = {0};

  for (uint16_t d = 1; d <= COUNT_NUMBER; d++)
  {
    BUFF.LRGB.L = (0B11100000) | (d * 2 / 3);
    BrightnessMatrix[d] = BUFF.DATA;
  }
  BrightnessMatrix[0] = 0;
  BrightnessMatrix[COUNT_NUMBER + 1] = UINT32_MAX;
  frame_queue = xQueueCreate(20, sizeof(screen_event_t));
  //初始化帧更新任务
  xTaskCreatePinnedToCore(vFrameUpData, "vFrameUpData", 2048, NULL, 1, NULL, 1);
  //启动定时器
  //vScreenShutDownDisplay();
  frameUpDataApp = ShowVoid;

  //vClockInit(screen_display_buff);
#if XXH_DEBUG
  //模拟转速传感器
  fps_test_timer_handle = xTimerCreate(NULL, (100), pdTRUE, NULL, vFpsTestCb);
  xTimerStart(fps_test_timer_handle, 0);
#else

  //初始化转速传感IO
  vSpeedSensorInit();
#endif
}

void RegisterPlayer(frameUpDataFunType fun)
{
  frameUpDataApp = fun;
}