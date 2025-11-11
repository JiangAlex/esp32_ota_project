#include "SystemView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>

SystemView::SystemView() : screen(nullptr), contentLabel(nullptr), statusBar(nullptr), created(false), scrollOffset(0) {}

SystemView::~SystemView() {
    destroy();
}

void SystemView::create() {
    if (created) return;
    
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    // 禁用滾動條
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // 創建統一的狀態欄 (16px 高度)
    statusBar = OLEDLayout::createStatusBar(screen);
    
    // 創建內容區域（48px 高度，從狀態欄下方開始）
    lv_obj_t* contentArea = lv_obj_create(screen);
    lv_obj_set_size(contentArea, 128, 48);
    lv_obj_set_pos(contentArea, 0, 16);
    lv_obj_set_style_bg_color(contentArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(contentArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(contentArea, 0, 0);
    lv_obj_set_style_pad_all(contentArea, 2, 0);
    lv_obj_set_scrollbar_mode(contentArea, LV_SCROLLBAR_MODE_OFF);
    
    // 創建內容標籤
    contentLabel = lv_label_create(contentArea);
    lv_label_set_text(contentLabel, "SYSTEM SETTINGS\n\nDevice Info:\n> Model: ESP32-WT\n> Firmware: v1.0.0\n> Memory: 4MB Flash\n> WiFi: 802.11 b/g/n\n\nConfiguration:\n> Language: English\n> Time Zone: UTC+8\n> Auto Sleep: 5min\n> Backlight: Auto\n\nConnectivity:\n> WiFi: Enabled\n> Bluetooth: Disabled\n> MQTT: Connected\n\nMaintenance:\n> Factory Reset\n> Update Firmware\n> Export Logs\n\nPress OK for scroll\nUP/DOWN to navigate");
    lv_obj_set_style_text_color(contentLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(contentLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_align(contentLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 2, 0);
    
    scrollOffset = 0;
    created = true;
    Serial.println("System View created");
}

void SystemView::destroy() {
    if (!created) return;
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
    contentLabel = nullptr;
    created = false;
    Serial.println("System View destroyed");
}

lv_obj_t* SystemView::getScreen() const {
    return screen;
}

bool SystemView::isCreated() const {
    return created;
}

void SystemView::scrollUp() {
    if (!created || !contentLabel) return;
    
    scrollOffset -= 5;
    if (scrollOffset < -20) {
        scrollOffset = -20; // 最大向上滾動限制
    }
    
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 2, scrollOffset);
    Serial.printf("SystemView: Scroll up to offset %d\n", scrollOffset);
}

void SystemView::scrollDown() {
    if (!created || !contentLabel) return;
    
    scrollOffset += 5;
    if (scrollOffset > 50) {
        scrollOffset = 50; // 最大向下滾動限制
    }
    
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 2, scrollOffset);
    Serial.printf("SystemView: Scroll down to offset %d\n", scrollOffset);
}

void SystemView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
    }
}