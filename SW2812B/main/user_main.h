#ifndef __USER_MAIN__
#define __USER_MAIN__
#include "button/button.h"
#include "sk9822_drive/sk9822.h"

#include "screen_drive/screen_drive.h"
#include "wifi/xxh_wifi.h"
#include "mqtt/xxh_mqtt.h"
#include "tf/tf.h"
#include "LZ77/LZ77.H"
#include "playmv/playmv.H"

void vShowTaskList(void);
void vButtonInit(void);
float getCellVoltage(void);
void vAdcInit(void);
void test_led(void);
#endif