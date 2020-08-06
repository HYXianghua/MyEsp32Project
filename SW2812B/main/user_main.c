#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "user_main.h"

void app_main()
{
  vDriveWifiInit();
  vMqttAppStart();
  //vShowTaskList();
  vAdcInit();
  vButtonInit();
  vDriveSk9822Init();
  //test_led();

  vTfInit();
  vScreenInit();

  for (;;)
  {
    vTaskDelay(1000);
  }
}
