#ifndef __USER_MAIN__
#define __USER_MAIN__
#include "button/button.h"
#include "sk9822_drive/sk9822.h"

#include "screen_drive/screen_drive.h"
#include "wifi/xxh_wifi.h"
#include "mqtt/xxh_mqtt.h"
#include "driver/mcpwm.h"

void vShowTaskList(void);
void vButtonInit(void);
#endif