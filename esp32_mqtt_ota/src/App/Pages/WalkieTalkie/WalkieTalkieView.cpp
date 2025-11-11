#include "WalkieTalkieView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>

WalkieTalkieView::WalkieTalkieView() : screen(nullptr), contentLabel(nullptr), statusBar(nullptr), created(false), scrollOffset(0) {}

WalkieTalkieView::~WalkieTalkieView() {
    destroy();
}

void WalkieTalkieView::create() {
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
    lv_label_set_text(contentLabel, "WALKIE-TALKIE\n\nChannels:\n1. Emergency\n2. Team Alpha\n3. Team Beta\n4. Command\n5. Supply\n\nSettings:\n> Volume: 8/10\n> Squelch: Level 3\n> Power: High\n> VOX: Enabled\n\nFunctions:\n> Push-to-Talk\n> Channel Scan\n> CTCSS Tones\n> Battery Monitor\n\nPress OK for scroll\nUP/DOWN to navigate");
    lv_obj_set_style_text_color(contentLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(contentLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_align(contentLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 2, 0);
    
    scrollOffset = 0;
    created = true;
    Serial.println("WalkieTalkie View created");
}

void WalkieTalkieView::destroy() {
    if (!created) return;
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
    contentLabel = nullptr;
    created = false;
    Serial.println("WalkieTalkie View destroyed");
}

lv_obj_t* WalkieTalkieView::getScreen() const {
    return screen;
}

bool WalkieTalkieView::isCreated() const {
    return created;
}

void WalkieTalkieView::scrollUp() {
    if (!created || !contentLabel) return;
    
    scrollOffset -= 5;
    if (scrollOffset < -20) {
        scrollOffset = -20; // 最大向上滾動限制
    }
    
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 2, scrollOffset);
    Serial.printf("WalkieTalkieView: Scroll up to offset %d\n", scrollOffset);
}

void WalkieTalkieView::scrollDown() {
    if (!created || !contentLabel) return;
    
    scrollOffset += 5;
    if (scrollOffset > 50) {
        scrollOffset = 50; // 最大向下滾動限制
    }
    
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 2, scrollOffset);
    Serial.printf("WalkieTalkieView: Scroll down to offset %d\n", scrollOffset);
}

void WalkieTalkieView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
    }
}