/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef ESPNOW_EXAMPLE_H
#define ESPNOW_EXAMPLE_H

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */
#if CONFIG_STATION_MODE
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF ESP_IF_WIFI_AP
#endif

#define ESPNOW_QUEUE_SIZE 6

#define IS_BROADCAST_ADDR(addr) (memcmp(addr, s_example_broadcast_mac, ESP_NOW_ETH_ALEN) == 0)

typedef enum
{
    EXAMPLE_ESPNOW_SEND_CB,
    EXAMPLE_ESPNOW_RECV_CB,
} example_espnow_event_id_t;

typedef struct
{
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} example_espnow_event_send_cb_t;

typedef struct
{
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    uint8_t *data;
    int data_len;
} example_espnow_event_recv_cb_t;

typedef union {
    example_espnow_event_send_cb_t send_cb;
    example_espnow_event_recv_cb_t recv_cb;
} example_espnow_event_info_t;

/* 调用ESPNOW发送或接收回调函数时，将事件发送到ESPNOW任务。 */
typedef struct
{
    example_espnow_event_id_t id;
    example_espnow_event_info_t info;
} example_espnow_event_t;

enum
{
    EXAMPLE_ESPNOW_DATA_BROADCAST,
    EXAMPLE_ESPNOW_DATA_UNICAST,
    EXAMPLE_ESPNOW_DATA_MAX,
};

/* 在这个例子中，用户定义了ESPNOW数据字段。 */
typedef struct
{
    uint8_t type;       // 广播或单播ESPNOW数据。Broadcast or unicast ESPNOW data.
    uint8_t state;      // 表示是否收到广播ESPNOW数据。Indicate that if has received broadcast ESPNOW data or not.
    uint16_t seq_num;   // ESPNOW数据的序列号。Sequence number of ESPNOW data.
    uint16_t crc;       // ESPNOW数据的CRC16值。CRC16 value of ESPNOW data.
    uint32_t magic;     // 用来决定哪个设备发送单播ESPNOW数据。Magic number which is used to determine which device to send unicast ESPNOW data.
    uint8_t payload[0]; // ESPNOW数据的实际有效载荷。Real payload of ESPNOW data.
} __attribute__((packed)) example_espnow_data_t;

/* 参数发送ESPNOW数据。 */
typedef struct
{
    bool unicast;                       // 发送单播ESPNOW数据。
    bool broadcast;                     // 发送广播ESPNOW数据。
    uint8_t state;                      // 表示是否收到广播ESPNOW数据。
    uint32_t magic;                     // 用来决定哪个设备发送单播ESPNOW数据。
    uint16_t count;                     // 要发送的单播ESPNOW数据的总数。
    uint16_t delay;                     // 发送两个ESPNOW数据之间的延迟，单位:ms。
    int len;                            // 要发送的ESPNOW数据的长度，单位:字节。
    uint8_t *buffer;                    // 指向ESPNOW数据的缓冲区。
    uint8_t dest_mac[ESP_NOW_ETH_ALEN]; // 目标设备的MAC地址。
} example_espnow_send_param_t;

#endif
