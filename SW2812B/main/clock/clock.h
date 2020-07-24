#ifndef __XXH_CLOCK_H__
#define __XXH_CLOCK_H__
#include "../Screen_drive/Screen_drive.h"

void vClockInit(frame_type *pDisplayBuff);
void vClockUpData(frame_type *p, uint16_t H, uint16_t M, uint16_t S);
#endif