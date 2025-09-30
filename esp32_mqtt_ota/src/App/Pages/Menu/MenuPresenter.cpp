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
    Serial.printf("Executing menu item: %s\n", item.text);
}