#ifndef SK9822_CONTROL_H
#define SK9822_CONTROL_H
#include <stdint.h>

// 初始化相关设备
void sk9822_drive_init(void);

void sk9822_sand_data(const uint32_t *data, int len);

#endif