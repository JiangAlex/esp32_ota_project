#include "MenuPresenter.h"
#include <Arduino.h>

MenuPresenter::MenuPresenter(MenuView* view, MenuModel* model) 
    : BasePresenter(view), model(model) {}

MenuPresenter::~MenuPresenter() {}

void MenuPresenter::onCreate() {
    Serial.println("Menu Presenter onCreate");
    view->create();
    // MainMenu 不再需要初始化選擇
}

void MenuPresenter::onDestroy() {
    Serial.println("Menu Presenter onDestroy");
    view->destroy();
}

void MenuPresenter::onShow() {
    Serial.println("Menu displayed");
    // MainMenu 不再需要選擇狀態更新
}

void MenuPresenter::onHide() {
    Serial.println("Menu hidden");
}

void MenuPresenter::selectMenuItem(int index) {
    model->setSelectedIndex(index);
    view->updateSelection(index);  // This method exists in the updated MenuView
    Serial.printf("Menu icon %d selected\n", index);
}

void MenuPresenter::executeSelectedItem() {
    const MenuItem& item = model->getSelectedItem();
    Serial.printf("Executing menu item: %s (ID: %d)\n", item.text, item.id);
    
    // 根據選中項目的ID執行對應動作
    // 這將由main.cpp中的按鍵處理器調用PageManager進行頁面切換
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

int MenuPresenter::getCurrentSelection() const {
    return model->getSelectedIndex();
}

void MenuPresenter::handleUpButton() {
    Serial.println("Menu: UP button - No action (handled by main.cpp)");
    // UP 按鍵由 main.cpp 直接處理頁面跳轉
}

void MenuPresenter::handleDownButton() {
    Serial.println("Menu: DOWN button - No action (handled by main.cpp)");
    // DOWN 按鍵由 main.cpp 直接處理頁面跳轉
}

void MenuPresenter::updateDisplay() {
    if (view) {
        // 更新時間和日期顯示
        view->updateTimeAndDate();
        // 更新電池電量顯示
        view->updateBatteryLevel();
        Serial.println("Menu: Display updated (time, date, battery)");
    }
}