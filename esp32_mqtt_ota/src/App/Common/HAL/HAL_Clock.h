#pragma once
#include <stdint.h>

namespace HAL {
    void Clock_Init();
    void Clock_GetInfo(/* ... */);
    void Clock_SetInfo(/* ... */);
    void Clock_GetTimeString(char* timeStr, char* battStr);
    void GetTimeAndBattery(char* timeStr, char* battStr);
}
