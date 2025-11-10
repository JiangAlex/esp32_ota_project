#ifndef STATUS_MODEL_H
#define STATUS_MODEL_H

#include "../../MVP/BaseModel.h"
#include <cstddef>

class StatusModel : public BaseModel {
private:
    unsigned long uptime;
    size_t freeMemory;
    size_t totalMemory;
    bool gpio32Status;
    bool gpio33Status;
    bool gpio34Status;

public:
    StatusModel();
    
    void updateSystemInfo();
    
    // Getters
    unsigned long getUptime() const { return uptime; }
    size_t getFreeMemory() const { return freeMemory; }
    size_t getTotalMemory() const { return totalMemory; }
    bool getGpio32Status() const { return gpio32Status; }
    bool getGpio33Status() const { return gpio33Status; }
    bool getGpio34Status() const { return gpio34Status; }
    
    // 格式化方法
    void getUptimeFormatted(char* buffer, size_t bufferSize) const;
    void getMemoryFormatted(char* buffer, size_t bufferSize) const;
    void getGpioStatusFormatted(char* buffer, size_t bufferSize) const;
};

#endif // STATUS_MODEL_H