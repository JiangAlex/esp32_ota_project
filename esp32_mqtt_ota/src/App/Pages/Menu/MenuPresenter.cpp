#include "MenuPresenter.h"
#include <Arduino.h>
#include "../../Utils/PageManager/PageManager.h"
#include "../../Common/HAL/HAL.h"

MenuPresenter::MenuPresenter(MenuView* view, MenuModel* model) 
    : BasePresenter(view), model(model) {}

MenuPresenter::~MenuPresenter() {}

void MenuPresenter::onCreate() {
    Serial.println("Menu Presenter onCreate");
    view->create();
    // MainMenu 不再需要初始化選擇
    // 初始化選單超時/狀態
    selectionActive = false;
    lastActivityMs = millis();
    // 初始化 OLED 自動關閉功能
    displayOn = true;
    lastDisplayActivityMs = millis();
}

void MenuPresenter::onDestroy() {
    Serial.println("Menu Presenter onDestroy");
    view->destroy();
}

void MenuPresenter::onShow() {
    Serial.println("Menu displayed");
    // MainMenu 不再需要選擇狀態更新
    // 每次顯示 MainMenu，恢復時間顯示狀態
    selectionActive = false;
    if (view) view->hideMenuOptions();
    lastActivityMs = millis();
    // 重置顯示狀態
    displayOn = true;
    lastDisplayActivityMs = millis();
    turnOnDisplay();
}

void MenuPresenter::onHide() {
    Serial.println("Menu hidden");
}

void MenuPresenter::selectMenuItem(int index) {
    model->setSelectedIndex(index);
    view->updateSelection(index);  // This method exists in the updated MenuView
    Serial.printf("Menu icon %d selected\n", index);
    // user activity
    lastActivityMs = millis();
    lastDisplayActivityMs = millis();
}

void MenuPresenter::executeSelectedItem() {
    const MenuItem& item = model->getSelectedItem();
    Serial.printf("Executing menu item: %s (ID: %d)\n", item.text, item.id);
    // 根據選中項目的ID執行對應動作
    PageManager* pm = PageManager::getInstance();
    if (!pm) return;
    switch (item.id) {
        case 0: pm->switchToPage(PAGE_TREKKING); break;
        case 1: pm->switchToPage(PAGE_WALKIETALKIE); break;
        case 2: pm->switchToPage(PAGE_STATUS); break;
        case 3: pm->switchToPage(PAGE_SYSTEM); break;
        default: break;
    }
    // exiting menu
    selectionActive = false;
    if (view) view->hideMenuOptions();
}

void MenuPresenter::moveUp() {
    int currentIndex = model->getSelectedIndex();
    int newIndex = (currentIndex > 0) ? currentIndex - 1 : model->getMenuItemCount() - 1;
    selectMenuItem(newIndex);
}

void MenuPresenter::moveDown() {
    int currentIndex = model->getSelectedIndex();
    int newIndex = (currentIndex + 1) % model->getMenuItemCount();
    selectMenuItem(newIndex);
}

void MenuPresenter::onOkPressed() {
    // If display is off, turn it on and show time display
    if (!displayOn) {
        turnOnDisplay();
        Serial.println("Menu: OK button - Turn on display");
        return;
    }
    
    // If menu is not active, enter selection mode
    if (!selectionActive) {
        selectionActive = true;
        if (view) view->showMenuOptions();
        lastActivityMs = millis();
        lastDisplayActivityMs = millis();
        Serial.println("Menu: Enter selection mode");
        return;
    }
    // If selectionActive, confirm current selection
    executeSelectedItem();
}

void MenuPresenter::onUserActivity() {
    lastActivityMs = millis();
    lastDisplayActivityMs = millis();
}

bool MenuPresenter::isSelectionActive() const {
    return selectionActive;
}

int MenuPresenter::getCurrentSelection() const {
    return model->getSelectedIndex();
}

void MenuPresenter::handleUpButton() {
    // If display is off, turn it on and show time display
    if (!displayOn) {
        turnOnDisplay();
        Serial.println("Menu: UP button - Turn on display");
        return;
    }
    
    // If menu is not active, enter selection mode
    if (!selectionActive) {
        selectionActive = true;
        if (view) view->showMenuOptions();
        lastActivityMs = millis();
        lastDisplayActivityMs = millis();
        Serial.println("Menu: UP button - Enter selection mode");
        return;
    }
    // If selectionActive, move up in menu
    moveUp();
    Serial.println("Menu: UP button - Move up in menu");
}

void MenuPresenter::handleDownButton() {
    // If display is off, turn it on and show time display
    if (!displayOn) {
        turnOnDisplay();
        Serial.println("Menu: DOWN button - Turn on display");
        return;
    }
    
    // If menu is not active, enter selection mode
    if (!selectionActive) {
        selectionActive = true;
        if (view) view->showMenuOptions();
        lastActivityMs = millis();
        lastDisplayActivityMs = millis();
        Serial.println("Menu: DOWN button - Enter selection mode");
        return;
    }
    // If selectionActive, move down in menu
    moveDown();
    Serial.println("Menu: DOWN button - Move down in menu");
}

void MenuPresenter::updateDisplay() {
    if (view) {
        // 只有在顯示開啟時才更新顯示內容
        if (displayOn) {
            // 更新時間和日期顯示
            view->updateTimeAndDate();
            // 更新電池電量顯示
            view->updateBatteryLevel();
            Serial.println("Menu: Display updated (time, date, battery)");
        }
        
        // 檢查是否在選單模式與超時 (10s) - 超時退回時間畫面而非進入頁面
        if (selectionActive) {
            if (millis() - lastActivityMs >= 10000UL) {
                Serial.println("Menu: selection timeout -> return to time display");
                selectionActive = false;
                if (view) view->hideMenuOptions();
                lastDisplayActivityMs = millis(); // 重置顯示活動時間
            }
        }
        
        // 檢查顯示自動關閉 (10秒無活動後關閉OLED)
        if (displayOn && !selectionActive) { // 只有在時間顯示模式下才自動關閉
            if (millis() - lastDisplayActivityMs >= 10000UL) {
                Serial.println("Menu: Display timeout (10s) -> Turn off OLED");
                turnOffDisplay();
            }
        }
    }
}

// OLED 控制方法實現
void MenuPresenter::turnOffDisplay() {
    if (!displayOn) return;
    
    displayOn = false;
    
    // 真正關閉 OLED 電源 - 進入睡眠模式
    HAL::Display_Sleep();
    
    Serial.println("Menu: OLED display turned OFF (sleep mode)");
}

void MenuPresenter::turnOnDisplay() {
    if (displayOn) return;
    
    displayOn = true;
    lastDisplayActivityMs = millis();
    
    // 真正開啟 OLED 電源 - 從睡眠模式喚醒
    HAL::Display_Wakeup();
    
    // 強制重新創建和顯示時間界面
    if (view) {
        view->updateTimeAndDate();
        view->updateBatteryLevel();
    }
    
    Serial.println("Menu: OLED display turned ON (wakeup from sleep)");
}

bool MenuPresenter::isDisplayOn() const {
    return displayOn;
}