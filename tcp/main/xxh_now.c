
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "freertos/FreeRTOS.h"

#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "rom/crc.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"

#include "xxh_now.h"
#include "app_main.h"

#define CHECK_ADDR(addrA, addrB) (memcmp(addrA, addrB, ESP_NOW_ETH_ALEN) == 0)

#define
typedef enum
{
    XXH_ESPNOW_SEND_CB,
    XXH_ESPNOW_RECV_CB,
} xnow_event_id_t;

enum
{
    XXH_ESPNOW_DATA_BROADCAST,
    XXH_ESPNOW_DATA_UNICAST,
    XXH_ESPNOW_DATA_MAX,
};

/* 用户定义ESPNOW数据字段. */
typedef struct xnow_data_t
{
    uint8_t ucXnowId;     // Xnow识别码
    uint8_t ucUserId;     // 用户的id识别码
    uint8_t ucDataId;     // 本数据的id识别码
    uint8_t ucSeqNum;     // ESPNOW数据的序列号.
    uint8_t ucTotalLen;   // ESPNOW数据的总数.
    uint8_t ucMagic;      // 高4位为发送方id,低四位为接收方id,0为主机 FF为广播
    uint16_t usCrc;       // ESPNOW数据的CRC16值.
    uint8_t ucPayload[0]; // ESPNOW数据的实际有效载荷.
} __attribute__((packed)) xnow_data_t;
//   uint8_t ucType;       // 广播或单播ESPNOW数据.

/* 参数发送ESPNOW数据. */
typedef struct xnow_send_param_t
{
    uint8_t ucMagic;        // 目标设备.
    uint8_t sand_data_len;  // 准备好的数据的长度，单位:字节.
    uint8_t data_id;        // 本次数据的id
    uint16_t usCount;       // 数据包数量.
    uint16_t usDelay;       // 发送两个ESPNOW数据之间的延迟，单位:ms.
    uint16_t buff_data_len; // 需要发送的数据的长度，单位:字节.
    uint8_t *sand_data;     // 指向准备好的数据.
    uint8_t *ucpData;       // 指向需要发送的数据
} xnow_send_param_t;

typedef struct device_control
{
    char name;
    uint8_t available_control;
} device_control_t;

typedef struct device_dev
{
    device_control_t available_control[6];
} device_dev_t;

typedef struct xxh_device
{
    uint8_t id;
    uint8_t type;
    device_dev_t dev;
} xxh_device_t;

typedef struct xxh_device_s_t
{
    uint8_t p;
    xxh_device_t xxh_device[XXHNOW_DEVICE_S_SIZE];
} xxh_device_s_t;

typedef struct xxh_magic_t
{
    uint8_t sender_id;
    uint8_t receiver_id;
} xxh_magic_t;

typedef struct xnow_event_send_cb_t
{
    esp_now_send_status_t status;
} xnow_event_send_cb_t;

typedef struct xnow_event_recv_cb_t
{
    uint8_t *data;
    int data_len;
} xnow_event_recv_cb_t;

typedef union {
    xnow_event_send_cb_t send_cb;
    xnow_event_recv_cb_t recv_cb;
} xnow_event_info_t;

/* 调用ESPNOW发送或接收回调函数时，将事件发送到ESPNOW任务. */
typedef struct xnow_event_t
{
    xnow_event_id_t id;
    xnow_event_info_t info;
} xnow_event_t;

static const char *TAG = "[X_NOW]";

//回调管理消息队列
static xQueueHandle s_xnow_cb_queue;

//发送管理消息队列
static xQueueHandle s_xnow_sn_queue;

//发送管理标志位
static EventGroupHandle_t s_xnow_sn_group;

//本机信息
static xxh_device_t this = {0};
//主机群id
static uint8_t host_ID = 0;

//目标mac地址
static uint8_t s_xnow_broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

//设备池
static xxh_device_s_t xxh_device_s = {0};

//消息id记录
static uint32_t xnow_data_id_buff_pool[30];
//消息id记录池指针
static uint8_t xnow_data_id_buff_pool_p = 0;
//数据缓冲池
static uint8_t xnow_data_buff[208] = {0};

//espnow任务终止
static void xnow_deinit()
{
    vSemaphoreDelete(s_xnow_cb_queue);
    esp_now_deinit();
}

//分解magic
xxh_magic_t xnow_magic_analyze(uint8_t data)
{
    xxh_magic_t buff;
    buff.sender_id = data >> 4;
    buff.receiver_id = data & 0b00001111;
    return buff;
}

//合并magic
uint8_t xnow_magic_merge(xxh_magic_t data)
{
    return (uint8_t)(data.sender_id << 4 || data.receiver_id & 0b00001111);
}

//将数据id添加至池尾
void xnow_data_id_pool_append(xnow_data_t *data)
{
    uint8_t data_buff = data->ucDataId << 16 || data->ucSeqNum << 8 || data->ucMagic;
    xnow_data_id_buff_pool[xnow_data_id_buff_pool_p] = data_buff;
    xnow_data_id_buff_pool_p++;
    if (xnow_data_id_buff_pool_p == 30)
    {
        xnow_data_id_buff_pool_p = 0;
    }
}

//查找数据id池 存在则返回1 否则为0
int xnow_data_id_pool_find(xnow_data_t *data)
{
    uint8_t data_buff = data->ucDataId << 16 || data->ucSeqNum << 8 || data->ucMagic;
    for (uint8_t i = 0; i < 30; i++)
    {
        if (xnow_data_id_buff_pool[xnow_data_id_buff_pool_p] == data_buff)
        {
            return 1;
        }
    }
    return 0;
}

//返回设备地址 返回0则不存在
xxh_device_t *xnow_get_device_addr(const uint8_t id)
{
    for (uint8_t i = 1; i < XXHNOW_DEVICE_S_SIZE && i < xxh_device_s.p; i++)
    {
        if (xxh_device_s.xxh_device[i].id == id)
        {
            return &xxh_device_s.xxh_device[i];
        }
    }
    return NULL;
}

//注册设备
esp_err_t xnow_add_device()
{
    return ESP_OK;
}

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void xnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    xnow_event_t evt;
    xnow_event_send_cb_t *send_cb = &evt.info.send_cb;
    //空指针校验
    if (mac_addr == NULL)
    {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }
    //设置为send模式
    evt.id = XXH_ESPNOW_SEND_CB;
    send_cb->status = status;
    if (xQueueSend(s_xnow_cb_queue, &evt, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGW(TAG, "Send send queue fail");
    }
}

static void xnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    xnow_event_t evt;
    xnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

    if (mac_addr == NULL || data == NULL || len <= 0)
    {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }
    // 设置广播地址
    evt.id = XXH_ESPNOW_RECV_CB;
    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL)
    {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }

    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(s_xnow_cb_queue, &evt, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }
}

/* Parse received ESPNOW data. 校验数据 */
esp_err_t xnow_data_parse(xnow_data_t *ucpData, uint16_t usDataLen)
{
    uint16_t usDataCrcBuff, usRealDataCrc = 0;

    if (usDataLen < sizeof(xnow_data_t))
    {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", usDataLen);
        return ESP_ERR_INVALID_SIZE;
    }
    if (ucpData->ucXnowId != XNOW_ID)
    {
        return ESP_ERR_INVALID_RESPONSE;
    }
    usDataCrcBuff = ucpData->usCrc;
    ucpData->usCrc = 0;
    usRealDataCrc = crc16_le(UINT16_MAX, (uint8_t const *)ucpData, usDataLen);

    if (usRealDataCrc == usDataCrcBuff)
    {
        return ESP_OK;
    }
    //buf->type
    return ESP_ERR_INVALID_CRC;
}

/* Prepare ESPNOW data to be sent. 准备数据 */
static esp_err_t xnow_data_prepare(xnow_send_param_t *send_data_buff, uint8_t seq_num)
{
    if (send_data_buff != NULL)
    {
        if (send_data_buff->data_id == 0) //首页初始化
        {
            //分配数据包id
            send_data_buff->data_id = (uint8_t)esp_random();
            //检查发送数据大小
            if (send_data_buff->buff_data_len == 0)
            {
                return ESP_ERR_INVALID_SIZE;
            }
            //计算包数量
            send_data_buff->usCount = (send_data_buff->buff_data_len - 1) / 200 + 1;
        }
        //检查页数大小
        if (seq_num > send_data_buff->usCount)
        {
            return ESP_ERR_INVALID_ARG;
        }
        //准备发送的数据包的指针
        send_data_buff->sand_data = xnow_data_buff;
        xnow_data_t *buf = (xnow_data_t *)send_data_buff->sand_data;
        //assert(send_param->len >= sizeof(xnow_data_t));
        //data_buff->ucType = IS_BROADCAST_ADDR(send_param->dest_mac) ? XXH_ESPNOW_DATA_BROADCAST : XXH_ESPNOW_DATA_UNICAST;
        buf->ucXnowId = XNOW_ID;
        buf->ucUserId = host_ID;
        buf->ucDataId = send_data_buff->data_id;
        buf->ucSeqNum = seq_num;
        //s_xnow_seq[buf->ucType]++;
        buf->ucTotalLen = send_data_buff->usCount;
        buf->ucMagic = send_data_buff->ucMagic;
        buf->usCrc = 0;
        send_data_buff->sand_data_len = 8;
        for (uint8_t i = 200 * (seq_num - 1); (i < (200 * (seq_num))) || (i < send_data_buff->buff_data_len); i++)
        {
            send_data_buff->sand_data_len++;
            buf->ucPayload[i] = *(send_data_buff->ucpData + i);
        }
        buf->usCrc = crc16_le(UINT16_MAX, (uint8_t const *)send_data_buff, send_data_buff->sand_data_len);
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_INVALID_ARG;
    }
}

static void xnow_cd_task(void *arg)
{
    xnow_event_t evt;
    vTaskDelay(5000 DMS);
    ESP_LOGI(TAG, "开始接收回调"); //Start sending broadcast data
    /* Start sending broadcast ESPNOW data. */
    xnow_send_param_t task_send_param;
    memset(&task_send_param, 0, sizeof(xnow_send_param_t));
    task_send_param.ucMagic = 0;
    task_send_param.usDelay = ESPNOW_SPEED;
    task_send_param.buff_data_len = 8;
    task_send_param.ucpData = (uint8_t *)TAG;
    ESP_LOGI(TAG, "xnow_data_prepare %x", xnow_data_prepare(&task_send_param, 1));

    if (esp_now_send(s_xnow_broadcast_mac, task_send_param.sand_data, task_send_param.sand_data_len) != ESP_OK)
    {
        ESP_LOGE(TAG, "Send error");
        xnow_deinit(&task_send_param);
        vTaskDelete(NULL);
    }

    while (xQueueReceive(s_xnow_cb_queue, &evt, portMAX_DELAY) == pdTRUE)
    {
        switch (evt.id)
        {
        case XXH_ESPNOW_SEND_CB:
        {
            xnow_event_send_cb_t *send_param_addr = &evt.info.send_cb;

            /* Delay a while before sending the next data. */

            ESP_LOGI(TAG, "广播数据 [state:%d]", send_param_addr->status);
            // /* 发送 Send the next data after the previous data is sent. */
            // esp_err_t err_buff = esp_now_send(s_xnow_broadcast_mac, send_param_addr->sand_data, send_param_addr->sand_data_len);
            // if (err_buff != ESP_OK)
            // {
            //     ESP_LOGE(TAG, "Send error:%d", err_buff);
            //     xnow_deinit();
            //     vTaskDelete(NULL);
            // }
            // if (send_param_addr->usDelay > 0)
            // {
            //     vTaskDelay(send_param_addr->usDelay DMS);
            // }
            break;
        }
        case XXH_ESPNOW_RECV_CB:
        {
            xnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
            xnow_data_t *recv_cb_data = (xnow_data_t *)recv_cb->data;
            //校对数据的crc以及xnow识别码
            if (xnow_data_parse(recv_cb_data, recv_cb->data_len) == ESP_OK)
            {
                //查找是否为重复数据
                if (xnow_data_id_pool_find(recv_cb_data) == 0)
                {
                    //添加到数据池
                    xnow_data_id_pool_append(recv_cb_data);
                    //校对用户群 校对消息接收id
                    if (recv_cb_data->ucUserId == host_ID && recv_cb_data->ucMagic == this.id)
                    {
                        //发送ACK
                    }
                    else
                    {
                        //转发
                        if (esp_now_send(s_xnow_broadcast_mac, (uint8_t *)recv_cb_data, recv_cb->data_len) != ESP_OK)
                        {
                            ESP_LOGE(TAG, "Send error");
                            xnow_deinit();
                            vTaskDelete(NULL);
                        }
                    }
                }
            }
            else
            {
                ESP_LOGI(TAG, "Receive error data from:");
                printf("data:[");
                for (uint8_t i = 0; i < recv_cb->data_len; i++)
                {
                    printf("%02X ", *(recv_cb->data + i));
                }
                printf("]\n");
            }
            break;
        }
        default:
            ESP_LOGE(TAG, "Callback type error: %d", evt.id);
            break;
        }
    }
}

static void xnow_sn_task(void *arg)
{
    xnow_send_param_t *evt;
    s_xnow_sn_group = xEventGroupCreate();

    while (xQueueReceive(s_xnow_sn_queue, &evt, portMAX_DELAY) == pdTRUE)
    {
        xnow_data_prepare(evt, 1);
        xEventGroupWaitBits(s_xnow_sn_group, CONNECTED_BIT, false, true,
                            portMAX_DELAY);
    }
}
esp_err_t vNowInit(void)
{
    //创建消息队列
    //回调管理消息队列
    s_xnow_cb_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(xnow_event_t));
    if (s_xnow_cb_queue == NULL)
    {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }
    //发送管理消息队列
    s_xnow_sn_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(xnow_send_param_t));
    if (s_xnow_sn_queue == NULL)
    {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    /* 初始化ESPNOW和注册发送和接收回调函数。*/
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(xnow_send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(xnow_recv_cb));

    /* pmk加密 */
    //ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK));

    /* 向对等列表中添加广播对等信息 Add broadcast peer information to peer list. */
    esp_now_peer_info_t peer = {0};
    peer.channel = XXHNOW_CHANNEL;
    peer.ifidx = ESPNOW_WIFI_IF;
    peer.encrypt = false;
    memcpy(&peer.peer_addr, s_xnow_broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    //设置本机信息
    this.id = 0;
    this.type = 0;
    memset(this.dev.available_control, 0, sizeof(device_dev_t));

    //本机mac地址
    uint8_t this_mac[6];
    esp_base_mac_addr_get(this_mac);

    //计算主机id
    host_ID = crc8_le(UINT8_MAX, this_mac, ESP_NOW_ETH_ALEN);

    //启动任务
    xTaskCreate(xnow_cd_task, "xnow_task", 2048, NULL, 4, NULL);

    return ESP_OK;
}

void vNowDataSend(void) //uint8_t target, uint8_t *data, uint8_t len)
{
    // xnow_send_param_t param_buff;
    // param_buff.sand_data;
    uint8_t buff[20] = {};
    memset(buff, 0xAA, 20);
    esp_now_send(s_xnow_broadcast_mac, buff, 20);
}
