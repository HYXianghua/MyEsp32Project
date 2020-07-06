/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "button/button.h"
#include "screen_drive/screen_drive.h"
#include "wifi/xxh_wifi.h"
#include "mqtt/xxh_mqtt.h"

#include "nvs_flash.h"
#include "driver/mcpwm.h"

static const char *TAG = "[X_MAIN]";

Button_t B1;
void Btn1_Dowm_CallBack(void *arg)
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
  ESP_LOGI("X_BNT", "B1_C\n");
}

static void task_list_task(void *pvParameters)
{
  uint8_t pcWriteBuffer[500];
  while (1)
  {
    vTaskDelay(5000 / portTICK_RATE_MS);
    vTaskList((char *)&pcWriteBuffer);
    ESP_LOGI(TAG,
             "%s%s%s\r\n%s",
             "----------------------------------------\r\n",
             "任务名      状态   优先级   剩余栈 任务序号\r\n",
             pcWriteBuffer,
             "----------------------------------------\r\n");
  }
}

void app_main()
{
  // nvs_flash_init();
  //xTaskCreatePinnedToCore(task_list_task, "task_list", 3072, NULL, 2, NULL, 0);
  // vWifiInit();
  // vMqttAppStart();
  //按键名字 按键句柄 按钮IO 触发电平
  Button_Create("B1", &B1, 2, 0);
  Button_Attach(&B1, BUTTON_DOWM, Btn1_Dowm_CallBack);
  but_init();

  vScreenInit();

  for (;;)
  {
    //sk9822_sand_data((uint8_t *)(&tese), sizeof(tese));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //vUpSpeed();
    //mqttSandRotateSpeed();
  }
}
