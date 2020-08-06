
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "user_main.h"

#include "nvs_flash.h"

#define SHOW_TASK_LIST 0

Button_t B1;
void Dowm_CB(void *arg)
{
  static uint8_t btn1_state = 0;
  if (btn1_state)
  {
    btn1_state = !btn1_state;
    vScreenOn();
  }
  else
  {
    btn1_state = !btn1_state;
    vScreenOff();
  }
  ESP_LOGD("[X_BNT]", "B1_C");
}

void test_led(void)
{
  const uint32_t a[] = {0,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001,
                        0b00001111000011110000111111100001};
  const uint32_t b[] = {0,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001,
                        0b11100001};
  for (;;)
  {
    sk9822_sand_data_len(a, 1, 26);
    sk9822_sand_data_len(a, 0, 26);
    vTaskDelay(1000);
    sk9822_sand_data_len(b, 1, 26);
    sk9822_sand_data_len(b, 0, 26);
    vTaskDelay(1000);
  }
}

static void task_list_task(void *pvParameters)
{
  uint8_t pcWriteBuffer[500];
  for (;;)
  {
    vTaskDelay(5000);
    vTaskList((char *)&pcWriteBuffer);
    ESP_LOGI("[T-LS]",
             "%s%s%s\r\n%s",
             "----------------------------------------\r\n",
             "任务名      状态   优先级   剩余栈 任务序号\r\n",
             pcWriteBuffer,
             "----------------------------------------\r\n");
  }
}

void vShowTaskList(void)
{
  xTaskCreatePinnedToCore(task_list_task, "task_list", 3072, NULL, 2, NULL, 0);
}

void vButtonInit(void)
{
  Button_t *B1 = pvPortMalloc(sizeof(Button_t));
  Button_Create("B1", B1, 0, 0);
  Button_Attach(B1, BUTTON_DOWM, Dowm_CB);
  vDriveButtonInit();
}
