#ifndef __XXH_MQTT_H__
#define __XXH_MQTT_H__

#define MYMQTT_HOST "192.168.150.1"
#define MYMQTT_PORT 1883
#define OPENUSER 0
#define USERNAME "hyxxh",
#define USERPASS "0129hyxxh",

#define SIGN_ID "xxhESP"
#define MODEL "PRAY"
#define ID "0"
#define TOPIC_RX "/myESP/DR"
#define TOPIC_TX "/myESP/DT"

void vMqttAppStart(void);
void mqttSandRotateSpeed(void);
#endif