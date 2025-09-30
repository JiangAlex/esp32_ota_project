#include "SettingsView.h"
#include <Arduino.h>

SettingsView::SettingsView() : screen(nullptr), created(false) {}

SettingsView::~SettingsView() {
    destroy();
}

void SettingsView::create() {
    if (created) return;
    
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x442200), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(title, 10, 10);
    
    lv_obj_t *info = lv_label_create(screen);
    lv_label_set_text(info, "WiFi Settings\nMQTT Settings\nDisplay Settings\nSystem Settings");
    lv_obj_set_style_text_color(info, lv_color_white(), 0);
    lv_obj_set_pos(info, 20, 60);
    
    created = true;
    Serial.println("Settings View created");
}

void SettingsView::destroy() {
    if (!created) return;
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
    created = false;
    Serial.println("Settings View destroyed");
}

lv_obj_t* SettingsView::getScreen() const {
    return screen;
}

bool SettingsView::isCreated() const {
    return created;
}