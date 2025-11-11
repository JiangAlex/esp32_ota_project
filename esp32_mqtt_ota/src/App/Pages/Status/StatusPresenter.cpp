#include "StatusPresenter.h"
#include <Arduino.h>

StatusPresenter::StatusPresenter(StatusView* view, StatusModel* model) 
    : BasePresenter(view), model(model) {}

StatusPresenter::~StatusPresenter() {}

void StatusPresenter::onCreate() {
    Serial.println("Status Presenter onCreate");
    view->create();
    updateStatus(); // 初始化狀態信息
}

void StatusPresenter::onDestroy() {
    Serial.println("Status Presenter onDestroy");
    view->destroy();
}

void StatusPresenter::onShow() {
    Serial.println("Status displayed");
    updateStatus(); // 顯示時更新狀態
}

void StatusPresenter::onHide() {
    Serial.println("Status hidden");
}

void StatusPresenter::updateStatus() {
    if (!view || !model) return;
    
    // 更新模型數據
    model->updateSystemInfo();
    
    // 格式化狀態信息
    char uptimeStr[32];
    char memoryStr[32];
    char gpioStr[64];
    char statusText[200];
    
    model->getUptimeFormatted(uptimeStr, sizeof(uptimeStr));
    model->getMemoryFormatted(memoryStr, sizeof(memoryStr));
    model->getGpioStatusFormatted(gpioStr, sizeof(gpioStr));
    
    snprintf(statusText, sizeof(statusText),
        "System: OK\n"
        "Memory: %s\n"
        "Uptime: %s\n"
        "GPIO: %s",
        memoryStr, uptimeStr, gpioStr
    );
    
    // 更新視圖
    view->updateStatus(statusText);
    
    Serial.printf("Status updated - %s, Memory: %s\n", uptimeStr, memoryStr);
}

void StatusPresenter::scrollUp() {
    if (view) {
        view->scrollUp();
    }
}

void StatusPresenter::scrollDown() {
    if (view) {
        view->scrollDown();
    }
}

void StatusPresenter::updateStatusBar(const char* batteryText, const char* timeText) {
    if (view) {
        view->updateStatusBar(batteryText, timeText);
    }
}