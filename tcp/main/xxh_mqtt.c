#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "xxh_mqtt.h"
#include "xxh_pt6311.h"

static const char *TAG = "[X_MQTT]";

static void vMqttCtrl(char *data)
{
  char cBuff[5] = "", cDev[10] = "", cData[10] = "";
  memset(cBuff, 0, 5);
  memset(cDev, 0, 10);
  memset(cData, 0, 10);
  sscanf(data, "%s%s%s", cBuff, cDev, cData);
  int ucABuff = 0, ucBBuff = 0;
  sscanf(cDev, "%d", &ucABuff);
  sscanf(cData, "%d", &ucBBuff);
  ESP_LOGI(TAG, "[c][%s][%s][%s]\n", cBuff, cDev, cData);
  vVfdChangeOptions(ucABuff, ucBBuff);
  ESP_LOGI(TAG, "[d][%s][%d][%d]\n", cBuff, ucABuff, ucBBuff);
}

static esp_err_t xMqttEventHandler(esp_mqtt_event_handle_t event)
{
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  // your_context_t *context = event->context;
  switch (event->event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
    // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
    // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
    // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data", 0, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
    ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);
    vMqttCtrl(event->data);

    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
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
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_start(client);
}