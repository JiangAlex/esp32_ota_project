#include "StatusView.h"
#include <Arduino.h>

StatusView::StatusView() : screen(nullptr), created(false), statusLabel(nullptr) {}

StatusView::~StatusView() {
    destroy();
}

void StatusView::create() {
    if (created) return;
    
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x004422), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "System Status");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(title, 10, 10);
    
    statusLabel = lv_label_create(screen);
    lv_label_set_text(statusLabel, "WiFi: Connecting...\nMQTT: Disconnected\nMemory: OK\nUptime: 00:00:00");
    lv_obj_set_style_text_color(statusLabel, lv_color_white(), 0);
    lv_obj_set_pos(statusLabel, 20, 60);
    
    created = true;
    Serial.println("Status View created");
}

void StatusView::destroy() {
    if (!created) return;
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
    statusLabel = nullptr;
    created = false;
    Serial.println("Status View destroyed");
}

lv_obj_t* StatusView::getScreen() const {
    return screen;
}

bool StatusView::isCreated() const {
    return created;
}

void StatusView::updateStatus(const char* status) {
    if (created && statusLabel) {
        lv_label_set_text(statusLabel, status);
    }
}