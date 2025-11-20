#pragma once
#include <stdint.h>

namespace HAL {
    struct Power_Info_t {
        float voltage;   // 電池電壓
        int percent;     // 電池百分比
        bool charging;   // 是否充電中
    };

    void Power_Init();
    void Power_GetInfo(Power_Info_t* info);
    int Power_GetPercent();
    float Power_GetVoltage();
    bool Power_IsCharging();
}
