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
    if (!view) return;
    
    // 使用 StatusView 自己的傳感器數據更新方法
    // 這會使用 DataProc 系統和 GY-80 傳感器數據
    view->updateSystemStatus();
    
    Serial.println("Status updated using GY-80 sensor data via DataProc");
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