#include "SettingsPresenter.h"
#include "SettingsModel.h"
#include <Arduino.h>

SettingsPresenter::SettingsPresenter(SettingsView* view, SettingsModel* model) : BasePresenter(view), model(model) {}

SettingsPresenter::~SettingsPresenter() {}

void SettingsPresenter::onCreate() {
    Serial.println("Settings Presenter onCreate");
    if (view && model) {
        view->create();
        updateSettingsDisplay();
    }
}

void SettingsPresenter::onDestroy() {
    // 清理資源
}

void SettingsPresenter::onShow() {
    if (view) {
        updateSettingsDisplay();
    }
    Serial.println("Settings displayed");
}

void SettingsPresenter::onHide() {
    Serial.println("Settings hidden");
}

void SettingsPresenter::updateSettingsDisplay() {
    if (view && model) {
        // SettingsView 的內容已經在 create() 中設置
        // 這裡可以添加額外的顯示更新邏輯
        Serial.println("Settings display updated");
    }
}

void SettingsPresenter::scrollUp() {
    if (model && view) {
        int currentIndex = model->getCurrentSelection();
        if (currentIndex > 0) {
            model->setCurrentSelection(currentIndex - 1);
            view->scrollUp();
        }
    }
}

void SettingsPresenter::scrollDown() {
    if (model && view) {
        int currentIndex = model->getCurrentSelection();
        int maxIndex = model->getSettingCount() - 1;
        if (currentIndex < maxIndex) {
            model->setCurrentSelection(currentIndex + 1);
            view->scrollDown();
        }
    }
}

void SettingsPresenter::selectSetting(int index) {
    if (model && index >= 0 && index < model->getSettingCount()) {
        model->setCurrentSelection(index);
        updateSettingsDisplay();
    }
}

void SettingsPresenter::executeSelectedSetting() {
    if (model && view) {
        int currentIndex = model->getCurrentSelection();
        const SettingItem* item = model->getSettingItem(currentIndex);
        
        if (item) {
            // 根據設定項目執行相應的操作
            switch (currentIndex) {
                case 0: // 亮度調節
                    // TODO: 實現亮度調節邏輯
                    break;
                case 1: // 自動關機
                    // TODO: 實現自動關機設定
                    break;
                case 2: // 時間設定
                    // TODO: 實現時間設定
                    break;
                case 3: // 日期設定
                    // TODO: 實現日期設定
                    break;
                case 4: // 語言設定
                    // TODO: 實現語言切換
                    break;
                case 5: // 重啟系統
                    ESP.restart();
                    break;
                case 6: // 恢復出廠設定
                    // TODO: 實現出廠設定恢復
                    break;
                case 7: // 關於系統
                    // TODO: 顯示關於頁面
                    break;
                default:
                    break;
            }
        }
    }
}