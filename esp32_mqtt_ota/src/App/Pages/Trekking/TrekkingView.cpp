#include "TrekkingView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>

TrekkingView::TrekkingView() : screen(nullptr), contentLabel(nullptr), statusBar(nullptr), created(false), scrollOffset(0) {
    Serial.println("TrekkingView: Constructor");
}

TrekkingView::~TrekkingView() {
    destroy();
    Serial.println("TrekkingView: Destructor");
}

void TrekkingView::create() {
    if (created) {
        Serial.println("TrekkingView: Already created");
        return;
    }
    
    Serial.println("TrekkingView: Creating screen");
    
    // 建立主螢幕
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen, lv_color_white(), LV_PART_MAIN);
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
    
    // 標題文字
    contentLabel = lv_label_create(contentArea);
    lv_obj_set_width(contentLabel, 124);
    lv_obj_set_style_text_align(contentLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_font(contentLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(contentLabel, lv_color_white(), 0);
    lv_obj_set_style_text_opa(contentLabel, LV_OPA_COVER, 0);
    lv_obj_set_style_outline_width(contentLabel, 0, 0);
    lv_obj_set_style_border_width(contentLabel, 0, 0);
    lv_obj_set_style_bg_opa(contentLabel, LV_OPA_TRANSP, 0);
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(contentLabel, "TREKKING MODE\n\nFeatures:\n> GPS Tracking\n> Distance Log\n> Speed Monitor\n> Route Planning\n> Waypoints\n> Elevation Data\n\nSettings:\n> Units: Metric\n> Auto-save: ON\n> Battery Save: OFF\n\nPress OK to toggle\nscroll mode\nUP/DOWN to scroll");
    
    scrollOffset = 0;
    created = true;
    Serial.println("TrekkingView: Created successfully");
}

void TrekkingView::destroy() {
    if (!created) {
        return;
    }
    
    Serial.println("TrekkingView: Destroying");
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
        contentLabel = nullptr; // 會被連帶刪除
    }
    
    created = false;
}

lv_obj_t* TrekkingView::getScreen() const {
    return screen;
}

bool TrekkingView::isCreated() const {
    return created;
}

void TrekkingView::scrollUp() {
    if (!created || !contentLabel) return;
    
    scrollOffset -= 5;
    if (scrollOffset < -20) {
        scrollOffset = -20; // 最大向上滾動限制
    }
    
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 0, scrollOffset);
    Serial.printf("TrekkingView: Scroll up to offset %d\n", scrollOffset);
}

void TrekkingView::scrollDown() {
    if (!created || !contentLabel) return;
    
    scrollOffset += 5;
    if (scrollOffset > 50) {
        scrollOffset = 50; // 最大向下滾動限制
    }
    
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 0, scrollOffset);
    Serial.printf("TrekkingView: Scroll down to offset %d\n", scrollOffset);
}

void TrekkingView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
    }
}