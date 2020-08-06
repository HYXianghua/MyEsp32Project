#ifndef SK9822_CONTROL_H
#define SK9822_CONTROL_H
#include <stdint.h>

// 初始化相关设备
void vDriveSk9822Init(void);

void sk9822_sand_data(const uint32_t *pData);
void sk9822_sand_data_len(const uint32_t *data, int dev, int len);
#endif