#ifndef __MY_ESPNOW_H__
#define __MY_ESPNOW_H__

/* 软路由或主机 station and softap mode*/
#if 1
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF ESP_IF_WIFI_AP
#endif

#define XNOW_ID 129             //Xnow的id识别码
#define XXHNOW_DEVICE_S_SIZE 20 //Xnow最大连接设备
#define ESPNOW_SPEED 50         //数据包发送之间的延迟
#define ESPNOW_QUEUE_SIZE 10    //消息队列大小
#define XXHNOW_CHANNEL 1        //now通道

//启动xxh_now
esp_err_t vNowInit(void);
void vNowDataSend(void);
#endif
