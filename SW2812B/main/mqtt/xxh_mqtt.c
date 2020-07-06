#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "xxh_mqtt.h"

//#include "driver/mcpwm.h"

static const char *TAG = "[X_MQTT]";
extern float rotate_speed;
esp_mqtt_client_handle_t client;

static void vMqttCtrl(char *data)
{
  // int ucABuff = 0;
  // sscanf(data, "%d", &ucABuff);
  // printf("%d", ucABuff);
  // //mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, ucABuff);

  // mcpwm_pin_config_t pin_config = {
  //     .mcpwm0a_out_num = 19};
  // mcpwm_set_pin(MCPWM_UNIT_0, &pin_config);
  // mcpwm_config_t pwm_config;
  // pwm_config.frequency = ucABuff; //frequency = 1000Hz
  // pwm_config.cmpr_a = 50.0;       //duty cycle of PWMxA = 60.0%
  // pwm_config.counter_mode = MCPWM_UP_COUNTER;
  // pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  // mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
  //strcmp
  // char cBuff[5] = "", cDev[10] = "", cData[10] = "";
  // memset(cBuff, 0, 5);
  // memset(cDev, 0, 10);
  // memset(cData, 0, 10);
  // sscanf(data, "%s%s%s", cBuff, cDev, cData);
  // int ucABuff = 0, ucBBuff = 0;
  // sscanf(cDev, "%d", &ucABuff);
  // sscanf(cData, "%d", &ucBBuff);
  // ESP_LOGI(TAG, "[c][%s][%s][%s]", cBuff, cDev, cData);
  // ESP_LOGI(TAG, "[d][%s][%d][%d]", cBuff, ucABuff, ucBBuff);
}

void mqttSandRotateSpeed()
{
  char buff[10];
  sprintf(buff, "%f\n", rotate_speed);
  esp_mqtt_client_publish(client, TOPIC_TX, buff, 10, 0, 0);
}

static esp_err_t xMqttEventHandler(esp_mqtt_event_handle_t event)
{
  esp_mqtt_client_handle_t client = event->client;
  // your_context_t *context = event->context;
  switch (event->event_id)
  {
  case MQTT_EVENT_CONNECTED: //连接
    ESP_LOGI(TAG, "连接");
    esp_mqtt_client_subscribe(client, TOPIC_RX, 0);
    break;

  case MQTT_EVENT_DISCONNECTED: //断开
    ESP_LOGI(TAG, "断开");
    break;

  case MQTT_EVENT_SUBSCRIBED: //订阅成功
    ESP_LOGI(TAG, "订阅成功, msg_id=%d", event->msg_id);
    esp_mqtt_client_publish(client, TOPIC_TX, "hello", 0, 0, 0);
    break;

  case MQTT_EVENT_UNSUBSCRIBED: //订阅失败
    ESP_LOGI(TAG, "订阅失败, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_PUBLISHED: //发布事件
    ESP_LOGI(TAG, "发布事件, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_DATA: //接收数据
    ESP_LOGI(TAG, "接收数据");
    ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
    ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
    vMqttCtrl(event->data);

    break;
  case MQTT_EVENT_ERROR: //错误
    ESP_LOGI(TAG, "错误");
    break;
  default:
    ESP_LOGI(TAG, "无定义 id:%d", event->event_id);
    break;
  }
  return ESP_OK;
}

void vMqttAppStart(void)
{
  esp_mqtt_client_config_t mqtt_cfg = {
    .host = MYMQTT_HOST,
    .event_handle = xMqttEventHandler,
    .port = MYMQTT_PORT
#if OPENUSER
    ,
    .username = USERNAME,
    .password = USERPASS,
#endif
  };
  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_start(client);
}