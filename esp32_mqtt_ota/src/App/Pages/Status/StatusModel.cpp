#include "StatusModel.h"
#include <Arduino.h>

StatusModel::StatusModel() : uptime(0), freeMemory(0), totalMemory(0), 
                           gpio32Status(true), gpio33Status(true), gpio34Status(false) {}

void StatusModel::updateSystemInfo() {
    // 更新系統運行時間
    uptime = millis();
    
    // 更新記憶體信息
    freeMemory = ESP.getFreeHeap();
    totalMemory = ESP.getHeapSize();
    
    // 更新GPIO狀態（這裡先設為固定值，實際應用中可以讀取真實狀態）
    gpio32Status = true;  // GPIO 32 通常工作正常
    gpio33Status = true;  // GPIO 33 通常工作正常  
    gpio34Status = false; // GPIO 34 需要外部上拉電阻
}

void StatusModel::getUptimeFormatted(char* buffer, size_t bufferSize) const {
    unsigned long uptimeSeconds = uptime / 1000;
    unsigned long hours = uptimeSeconds / 3600;
    unsigned long minutes = (uptimeSeconds % 3600) / 60;
    unsigned long seconds = uptimeSeconds % 60;
    
    snprintf(buffer, bufferSize, "%02lu:%02lu:%02lu", hours, minutes, seconds);
}

void StatusModel::getMemoryFormatted(char* buffer, size_t bufferSize) const {
    snprintf(buffer, bufferSize, "%dKB/%dKB", 
             (int)(freeMemory/1024), (int)(totalMemory/1024));
}

void StatusModel::getGpioStatusFormatted(char* buffer, size_t bufferSize) const {
    snprintf(buffer, bufferSize, "32:%s 33:%s 34:%s",
             gpio32Status ? "OK" : "--",
             gpio33Status ? "OK" : "--", 
             gpio34Status ? "OK" : "--");
}