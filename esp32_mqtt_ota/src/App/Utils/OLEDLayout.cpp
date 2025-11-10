#include "OLEDLayout.h"
#include <Arduino.h>

lv_obj_t* OLEDLayout::createTopBar(lv_obj_t* parent, const char* title) {
    // 創建頂部容器（單色 OLED 標題區域）
    lv_obj_t* topBar = lv_obj_create(parent);
    lv_obj_set_size(topBar, OLED_WIDTH, OLED_TOP_YELLOW_HEIGHT);
    lv_obj_set_pos(topBar, 0, 0);
    
    // 設置背景樣式
    lv_obj_set_style_bg_color(topBar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(topBar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(topBar, 0, 0);
    lv_obj_set_style_pad_all(topBar, 2, 0);
    
    // 創建標題標籤
    lv_obj_t* titleLabel = lv_label_create(topBar);
    lv_label_set_text(titleLabel, title);
    lv_obj_set_style_text_color(titleLabel, lv_color_white(), 0);
    lv_obj_center(titleLabel);
    
    // 添加狀態指示器（右側小點）
    lv_obj_t* statusDot = lv_obj_create(topBar);
    lv_obj_set_size(statusDot, 6, 6);
    lv_obj_set_pos(statusDot, OLED_WIDTH - 10, 5);
    lv_obj_set_style_bg_color(statusDot, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(statusDot, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(statusDot, 0, 0);
    lv_obj_set_style_radius(statusDot, 3, 0); // 圓形
    
    Serial.printf("OLED TopBar created: %s\n", title);
    return topBar;
}

lv_obj_t* OLEDLayout::createContentArea(lv_obj_t* parent) {
    // 創建內容區域（單色 OLED 內容區域）
    lv_obj_t* contentArea = lv_obj_create(parent);
    lv_obj_set_size(contentArea, OLED_WIDTH, OLED_BOTTOM_BLUE_HEIGHT);
    lv_obj_set_pos(contentArea, 0, OLED_TOP_YELLOW_HEIGHT);
    
    // 設置背景樣式
    lv_obj_set_style_bg_color(contentArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(contentArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(contentArea, 0, 0);
    lv_obj_set_style_pad_all(contentArea, 4, 0);
    
    // 禁用滾動條
    lv_obj_set_scrollbar_mode(contentArea, LV_SCROLLBAR_MODE_OFF);
    
    Serial.println("OLED ContentArea created");
    return contentArea;
}

void OLEDLayout::updateTopBarTitle(lv_obj_t* topBar, const char* newTitle) {
    if (!topBar) return;
    
    // 查找標題標籤（通常是第一個子對象）
    lv_obj_t* titleLabel = lv_obj_get_child(topBar, 0);
    if (titleLabel && lv_obj_check_type(titleLabel, &lv_label_class)) {
        lv_label_set_text(titleLabel, newTitle);
        lv_obj_center(titleLabel);
        Serial.printf("TopBar title updated: %s\n", newTitle);
    }
}

void OLEDLayout::applyOLEDStyle(lv_obj_t* obj) {
    if (!obj) return;
    
    // OLED優化樣式：高對比度、小字體
    lv_obj_set_style_text_color(obj, lv_color_white(), 0);
    lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_white(), 0);
}