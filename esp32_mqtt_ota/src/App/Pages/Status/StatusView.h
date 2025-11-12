#ifndef STATUS_VIEW_H
#define STATUS_VIEW_H

#include "../../MVP/BaseView.h"
#include "../../../lib/GY80/GY80.h"
#include "../../Common/DataProc/DataProc.h"

class StatusView : public BaseView {
private:
    lv_obj_t* screen;
    bool created;
    lv_obj_t* statusLabel;
    lv_obj_t* statusBar;  // 頂部狀態欄
    GY80::GY80 gy80;     // GY-80 傳感器實例
    
    // DataProc accounts for sensor data
    Account* sensorsAccount;     // 傳感器數據帳號
    Account* stepCounterAccount; // 計步器數據帳號
    
    // 傳感器狀態追蹤
    bool sensorsHardwareAvailable; // 硬體傳感器是否可用
    uint32_t lastSensorCheck;      // 上次檢查傳感器的時間

public:
    StatusView();
    ~StatusView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    
    void updateStatus(const char* status);
    void updateSystemStatus();
    void updateSensorStatus();  // 更新 GY-80 傳感器數據
    void scrollUp();
    void scrollDown();
    void updateStatusBar(const char* batteryText, const char* timeText);
    
private:
    float calculateAltitude(float pressure);  // 計算海拔高度 (使用標準海平面氣壓)
    float calculateAltitude(float pressure, float seaLevelPressure);  // 計算海拔高度 (指定海平面氣壓)
    float calculateHeading(float mx, float my);  // 計算指南針方向
    const char* getCompassDirection(float heading);  // 獲取方向字符串
    void updateStepCounter(float ax, float ay, float az);  // 更新計步器
    void publishSensorData();  // 發布傳感器數據到 DataProc
    void publishStepData();    // 發布計步器數據到 DataProc
    void recheckSensorHardware();  // 重新檢查傳感器硬體狀態
};

#endif // STATUS_VIEW_H