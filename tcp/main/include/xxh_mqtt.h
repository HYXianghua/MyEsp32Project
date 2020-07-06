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
#define TOPIC_RX "/myESP/Date/0"
#define TOPIC_TX "/myESP/Date/1"

typedef struct
{
    char id[3];
    char sing_id[10];
    char model[7];
    char check_data[20];
    int check_data_len;
    bool sing_up_state;
} xxhESP_ID;

typedef struct xxhDev
{
    char id[5];
    char model[7];
    char check_data[20];
    int check_data_len;
    struct xxhDev *next;
} xxhDevice;

void vMqttAppStart(void);
#endif