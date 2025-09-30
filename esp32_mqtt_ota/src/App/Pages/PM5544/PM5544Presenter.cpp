#include "PM5544Presenter.h"
#include <Arduino.h>

PM5544Presenter::PM5544Presenter(PM5544View* view, PM5544Model* model) 
    : BasePresenter(view), model(model) {}

PM5544Presenter::~PM5544Presenter() {}

void PM5544Presenter::onCreate() {
    Serial.println("PM5544 Presenter onCreate");
    view->create();
}

void PM5544Presenter::onDestroy() {
    Serial.println("PM5544 Presenter onDestroy");
    view->destroy();
}

void PM5544Presenter::onShow() {
    Serial.println("PM5544 Pattern displayed");
}

void PM5544Presenter::onHide() {
    Serial.println("PM5544 Pattern hidden");
}

void PM5544Presenter::refreshPattern() {
    if (view->isCreated()) {
        view->destroy();
        view->create();
        Serial.println("PM5544 Pattern refreshed");
    }
}