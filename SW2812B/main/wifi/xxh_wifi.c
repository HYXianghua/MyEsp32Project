#include <string.h>
#include <stdint.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "esp_sntp.h"
#include "nvs_flash.h"

#include "xxh_wifi.h"

static const char *TAG = "[X_WIFI]";

const static int CONNECTED_BIT = BIT0;
static EventGroupHandle_t wifi_event_group;

static esp_err_t xWifiEventHandler(void *ctx, system_event_t *event)
{
  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
    esp_wifi_connect();
    break;
  case SYSTEM_EVENT_STA_GOT_IP: //获取ip
    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED: //断开连接
    esp_wifi_connect();
    xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    break;
  default:
    break;
  }
  return ESP_OK;
}

void vWifiInitSntp(void)
{
  ESP_LOGI(TAG, "------------Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "ntp.aliyun.com"); //设置访问服务器	中国提供商
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

void vDriveWifiInit(void)
{
  nvs_flash_init();
  tcpip_adapter_init();
  //创建wifi事件
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_init(xWifiEventHandler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = MYWIFI_SSID,
              .password = MYWIFI_PASS,
          },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_LOGI(TAG, "start the WIFI SSID:[%s]", MYWIFI_SSID);
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "Waiting for wifi");
  xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true,
                      portMAX_DELAY);
  vWifiInitSntp();
}

// /* WiFi should start before using ESPNOW */
// static void xnow_wifi_init(void)
// {
//     tcpip_adapter_init();
//     ESP_ERROR_CHECK(esp_event_loop_init(xnow_wifi_event_handler, NULL));
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//     ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

//     ESP_ERROR_CHECK(esp_wifi_set_mode(ESPNOW_WIFI_MODE));
//     ESP_ERROR_CHECK(esp_wifi_start());

//     /* In order to simplify xxh, channel is set after WiFi started.
//      * This is not necessary in real application if the two devices have
//      * been already on the same channel.
//      */
//     //ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));
// }