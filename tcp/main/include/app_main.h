#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__

#ifdef INVSC
#include "build/config/sdkconfig.h"
#endif

#define DMS / portTICK_RATE_MS

void vMainStopSntp(void);
struct tm tmMainGetNowTime(void);

#endif
