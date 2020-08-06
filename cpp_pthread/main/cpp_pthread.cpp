/* pthread/std::thread example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <string>
#include <sstream>
#include <esp_pthread.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "driver/timer.h"

using namespace std::chrono;
static uint32_t ti;
static uint32_t s;
//定时器回调
void IRAM_ATTR vScreenEvtTimerIsrHandler(void *para)
{
  int timer_idx = (int)para;
  uint32_t intr_status = TIMERG0.int_st_timers.val;
  TIMERG0.hw_timer[timer_idx].update = 1;
  if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) //线位置定时器
  {
    s = 1;
    TIMERG0.int_clr_timers.t0 = 1;
  }
  TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;
}
#define FRAME_TIMER TIMER_GROUP_0, TIMER_0           //中断计时器
#define TIMER_DIVIDER 8                              //预分频
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) //实际频率

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
  timer_set_alarm_value(FRAME_TIMER, TIMER_SCALE / (192 * 24));
  timer_enable_intr(FRAME_TIMER);
  timer_isr_register(FRAME_TIMER, vScreenEvtTimerIsrHandler,
                     (void *)TIMER_0, ESP_INTR_FLAG_IRAM, NULL);
  timer_start(FRAME_TIMER);
}

const auto sleep_time = seconds{
    5};

void print_thread_info(const char *extra = nullptr)
{
  std::stringstream ss;
  if (extra)
  {
    ss << extra;
  }
  ss << "Core id: " << xPortGetCoreID()
     << ", prio: " << uxTaskPriorityGet(nullptr)
     << ", minimum free stack: " << uxTaskGetStackHighWaterMark(nullptr) << " bytes.";
  ESP_LOGI(pcTaskGetTaskName(nullptr), "%s", ss.str().c_str());
}

void spawn_another_thread()
{

  while (true)
  {
    std::stringstream ss;
    ss << "ti:" << ti;
    ESP_LOGI(pcTaskGetTaskName(nullptr), "%s", ss.str().c_str());
    ti = 0;

    std::this_thread::sleep_for(seconds{1});
  }
}

void thread_func_any_core()
{
  while (true)
  {

    print_thread_info("This thread (with the default name) may run on any core.");
    std::this_thread::sleep_for(sleep_time);
  }
}

void thread_func()
{
  vScreenEvtTimerInit();
  while (true)
  {
    if (s)
    {
      s = 0;
      ti++;
    }
  }
}

esp_pthread_cfg_t create_config(const char *name, int core_id, int stack, int prio)
{
  auto cfg = esp_pthread_get_default_config();
  cfg.thread_name = name;
  cfg.pin_to_core = core_id;
  cfg.stack_size = stack;
  cfg.prio = prio;
  return cfg;
}

extern "C" void app_main()
{
  // 使用可以在任何核心上运行的deafult值创建线程
  auto cfg = esp_pthread_get_default_config();
  esp_pthread_set_cfg(&cfg);
  std::thread any_core(thread_func_any_core);

  // Create a thread on core 0 that spawns another thread, they will both have the same name etc.
  cfg = create_config("Thread 1", 0, 3 * 1024, 5);
  cfg.inherit_cfg = true;
  esp_pthread_set_cfg(&cfg);
  std::thread thread_1(spawn_another_thread);

  // Create a thread on core 1.
  cfg = create_config("Thread 2", 1, 3 * 1024, 5);
  esp_pthread_set_cfg(&cfg);
  std::thread thread_2(thread_func);

  // Let the main task do something too
  while (true)
  {
    std::stringstream ss;
    ss << "core id: " << xPortGetCoreID()
       << ", prio: " << uxTaskPriorityGet(nullptr)
       << ", minimum free stack: " << uxTaskGetStackHighWaterMark(nullptr) << " bytes.";
    ESP_LOGI(pcTaskGetTaskName(nullptr), "%s", ss.str().c_str());
    std::this_thread::sleep_for(sleep_time);
  }
}
