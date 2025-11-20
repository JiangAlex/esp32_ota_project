#include "HAL_Power.h"
#include <Arduino.h>

namespace HAL {
// 分壓電路: 3.3V -> 9kΩ -> 36pin(ADC1_CH0) -> 200kΩ -> GND
#define BAT_ADC_PIN 36
#define BAT_ADC_CHANNEL ADC1_CHANNEL_0
#define BAT_ADC_ATTEN ADC_ATTEN_DB_11
#define BAT_ADC_WIDTH ADC_WIDTH_BIT_12

// 分壓比: Vout = Vin * (200k / (9k + 200k))
constexpr float R1 = 9000.0f;   // 9kΩ
constexpr float R2 = 200000.0f; // 200kΩ
constexpr float VOLTAGE_DIVIDER = (R2 / (R1 + R2));
constexpr float ADC_MAX = 4095.0f;
constexpr float REF_VOLTAGE = 3.3f;

static Power_Info_t powerInfo = {3.7f, 85, false}; // 預設值

void Power_Init() {
    // 初始化 ADC
    analogReadResolution(12); // 12-bit
    // 若用 ESP-IDF 可用 adc1_config_width, adc1_config_channel_atten
    Serial.println("Power module initialized.");
}

static float readBatteryVoltage() {
    int raw = analogRead(BAT_ADC_PIN);
    float vout = (raw / ADC_MAX) * REF_VOLTAGE;
    float vin = vout / VOLTAGE_DIVIDER;
    return vin;
}

static int voltageToPercent(float vin) {
    // 假設鋰電池 3.0V=0%, 4.2V=100%
    if (vin < 3.0f) return 0;
    if (vin > 4.2f) return 100;
    return (int)((vin - 3.0f) * 100.0f / (4.2f - 3.0f));
}

void Power_GetInfo(Power_Info_t* info) {
    if (!info) return;
    float vin = readBatteryVoltage();
    int percent = voltageToPercent(vin);
    // charging 狀態可根據 GPIO 或 PMIC 判斷，這裡預設 false
    info->voltage = vin;
    info->percent = percent;
    info->charging = false;
    powerInfo = *info;
}

int Power_GetPercent() {
    Power_Info_t info;
    Power_GetInfo(&info);
    return info.percent;
}

float Power_GetVoltage() {
    Power_Info_t info;
    Power_GetInfo(&info);
    return info.voltage;
}

bool Power_IsCharging() {
    Power_Info_t info;
    Power_GetInfo(&info);
    return info.charging;
}
}
