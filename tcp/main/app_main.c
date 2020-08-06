

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "nvs_flash.h"

#include "xxh_pt6311.h"
#include "xxh_gpio.h"
#include "xxh_wifi.h"
#include "xxh_mqtt.h"
#include "xxh_now.h"

#include "app_main.h"

static const char *TAG = "[X_MAIN]";

void vMainInitSntp(void)
{
  ESP_LOGI(TAG, "------------Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "cn.ntp.org.cn"); //设置访问服务器	中国提供商
  sntp_init();
  setenv("TZ", "CST-8", 1);
  tzset();
}
void vMainStopSntp(void) { sntp_stop(); }

struct tm tmMainGetNowTime(void)
{
  time_t now = 0;
  struct tm timeinfo = {0};
  time(&now);                   //获取网络时间， 64bit的秒计数
  localtime_r(&now, &timeinfo); //转换成具体的时间参数
  return timeinfo;
}

void app_main()
{
  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

  nvs_flash_init();
  //vVfdTaskInit();
  vVfdWifiLinking();
  vDriveWifiInit();
  vNowInit();
  //vMqttAppStart();
  //vMainInitSntp();
  //vVfdTimeTaskInit();
  vTaskDelay(2000 DMS);
  vNowDataSend();
  //
  for (;;)
  {
    char pcWriteBuffer[1000];
    vTaskList(pcWriteBuffer);
    printf("------------------------------------------\n%s", pcWriteBuffer);
    vTaskDelay(2000 DMS);
  }
}
