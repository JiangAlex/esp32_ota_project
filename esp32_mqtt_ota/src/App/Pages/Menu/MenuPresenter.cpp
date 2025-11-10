#include "MenuPresenter.h"
#include <Arduino.h>

MenuPresenter::MenuPresenter(MenuView* view, MenuModel* model) 
    : BasePresenter(view), model(model) {}

MenuPresenter::~MenuPresenter() {}

void MenuPresenter::onCreate() {
    Serial.println("Menu Presenter onCreate");
    view->create();
}

void MenuPresenter::onDestroy() {
    Serial.println("Menu Presenter onDestroy");
    view->destroy();
}

void MenuPresenter::onShow() {
    Serial.println("Menu displayed");
}

void MenuPresenter::onHide() {
    Serial.println("Menu hidden");
}

void MenuPresenter::selectMenuItem(int index) {
    model->setSelectedIndex(index);
    view->updateSelection(index);
    Serial.printf("Menu item %d selected: %s\n", index, model->getSelectedItem().text);
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