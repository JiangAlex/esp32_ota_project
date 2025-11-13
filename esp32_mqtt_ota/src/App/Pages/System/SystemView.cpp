#include "SystemView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>

SystemView::SystemView() : 
    screen(nullptr), statusBar(nullptr), contentArea(nullptr), 
    arrowIndicator(nullptr), hintLabel(nullptr), created(false), selectedItem(0) {
    // 初始化選單項目陣列
    for (int i = 0; i < 3; i++) {
        menuItems[i] = nullptr;
    }
}

SystemView::~SystemView() {
    destroy();
}

void SystemView::create() {
    if (created) return;
    
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // 創建統一的狀態欄 (16px 高度)
    statusBar = OLEDLayout::createStatusBar(screen);
    
    // 創建系統選單佈局
    createSystemMenuLayout();
    
    created = true;
    Serial.println("System View (Menu Mode) created");
}

void SystemView::destroy() {
    if (!created) return;
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
    statusBar = nullptr;
    contentArea = nullptr;
    arrowIndicator = nullptr;
    hintLabel = nullptr;
    for (int i = 0; i < 3; i++) {
        menuItems[i] = nullptr;
    }
    created = false;
    Serial.println("System View destroyed");
}

lv_obj_t* SystemView::getScreen() const {
    return screen;
}

bool SystemView::isCreated() const {
    return created;
}

// 選單導航方法
void SystemView::selectPrevItem() {
    selectedItem = (selectedItem - 1 + 3) % 3;  // 循環選擇 0-2
    updateArrowPosition();
    Serial.printf("SystemView: Selected item %d\n", selectedItem);
}

void SystemView::selectNextItem() {
    selectedItem = (selectedItem + 1) % 3;  // 循環選擇 0-2
    updateArrowPosition();
    Serial.printf("SystemView: Selected item %d\n", selectedItem);
}

void SystemView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
    }
}

// 創建系統選單佈局
void SystemView::createSystemMenuLayout() {
    // 創建內容區域（32px 高度，從狀態欄下方開始）
    contentArea = lv_obj_create(screen);
    lv_obj_set_size(contentArea, 128, 32);
    lv_obj_set_pos(contentArea, 0, 16);
    lv_obj_set_style_bg_color(contentArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(contentArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(contentArea, 0, 0);
    lv_obj_set_style_pad_all(contentArea, 2, 0);
    lv_obj_set_scrollbar_mode(contentArea, LV_SCROLLBAR_MODE_OFF);
    
    // 創建選單項目（3個項目，每個高度約10px）
    createMenuItem(0, "1 Display Brightness");
    createMenuItem(1, "2 Time Setting");
    createMenuItem(2, "3 Battery Calibration");
    
    // 創建箭頭指示器
    arrowIndicator = lv_label_create(contentArea);
    lv_label_set_text(arrowIndicator, ">");
    lv_obj_set_style_text_font(arrowIndicator, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(arrowIndicator, lv_color_white(), 0);
    lv_obj_set_pos(arrowIndicator, 2, 2);  // 初始位置在第一項
    
    // 創建底部提示區域（16px 高度）
    lv_obj_t* hintArea = lv_obj_create(screen);
    lv_obj_set_size(hintArea, 128, 16);
    lv_obj_set_pos(hintArea, 0, 48);
    lv_obj_set_style_bg_color(hintArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(hintArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hintArea, 0, 0);
    lv_obj_set_style_pad_all(hintArea, 2, 0);
    lv_obj_set_scrollbar_mode(hintArea, LV_SCROLLBAR_MODE_OFF);
    
    hintLabel = lv_label_create(hintArea);
    lv_label_set_text(hintLabel, "[OK] Enter [MENU] Exit");
    lv_obj_set_style_text_font(hintLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(hintLabel, lv_color_white(), 0);
    lv_obj_set_style_text_align(hintLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(hintLabel);
    
    // 初始化選中項目
    selectedItem = 0;
    updateDisplay();
}

// 創建單個選單項目
void SystemView::createMenuItem(int index, const char* itemText) {
    if (index >= 3) return;
    
    menuItems[index] = lv_label_create(contentArea);
    lv_obj_set_size(menuItems[index], 120, 10);
    lv_obj_set_pos(menuItems[index], 10, 2 + index * 10);  // 每項間距10px
    
    lv_label_set_text(menuItems[index], itemText);
    lv_obj_set_style_text_font(menuItems[index], &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(menuItems[index], lv_color_white(), 0);
    lv_obj_set_style_text_align(menuItems[index], LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_bg_opa(menuItems[index], LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(menuItems[index], 0, 0);
}

void SystemView::confirmSelection() {
    SystemItem selected = getSelectedItem();
    Serial.printf("SystemView: Confirmed selection %d\n", static_cast<int>(selected));
    
    switch (selected) {
        case SystemItem::BRIGHTNESS:
            Serial.println("Entering brightness adjustment");
            break;
        case SystemItem::TIME_SETTING:
            Serial.println("Entering time setting");
            break;
        case SystemItem::BATTERY_CAL:
            Serial.println("Starting battery calibration");
            break;
    }
}

// 更新顯示內容
void SystemView::updateDisplay() {
    if (!created) return;
    
    // 更新選單項目文字以顯示當前值
    if (menuItems[0]) {
        lv_label_set_text_fmt(menuItems[0], "1 Brightness: %s", getBrightnessText());
    }
    if (menuItems[1]) {
        lv_label_set_text_fmt(menuItems[1], "2 Time Format: %s", getTimeFormatText());
    }
    if (menuItems[2]) {
        lv_label_set_text_fmt(menuItems[2], "3 Battery: %s", getBatteryCalText());
    }
    
    updateArrowPosition();
}

// 更新箭頭位置
void SystemView::updateArrowPosition() {
    if (!arrowIndicator) return;
    
    int y = 2 + selectedItem * 10;  // 計算箭頭Y位置
    lv_obj_set_pos(arrowIndicator, 2, y);
}

// 輔助方法：獲取設定值的文字表示
const char* SystemView::getBrightnessText() const {
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d/10", settings.brightness);
    return buffer;
}

const char* SystemView::getTimeFormatText() const {
    return settings.timeFormat24h ? "24H" : "12H";
}

const char* SystemView::getBatteryCalText() const {
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d%%", settings.batteryLevel);
    return buffer;
}

// 設定調整方法
void SystemView::adjustBrightness(bool increase) {
    if (increase && settings.brightness < 10) {
        settings.brightness++;
    } else if (!increase && settings.brightness > 1) {
        settings.brightness--;
    }
    updateDisplay();
    Serial.printf("Brightness adjusted to: %d\n", settings.brightness);
}

void SystemView::toggleTimeFormat() {
    settings.timeFormat24h = !settings.timeFormat24h;
    updateDisplay();
    Serial.printf("Time format changed to: %s\n", settings.timeFormat24h ? "24H" : "12H");
}

void SystemView::calibrateBattery() {
    // 模擬電池校準
    settings.batteryLevel = 100;  // 重置為100%
    updateDisplay();
    Serial.println("Battery calibrated to 100%");
}