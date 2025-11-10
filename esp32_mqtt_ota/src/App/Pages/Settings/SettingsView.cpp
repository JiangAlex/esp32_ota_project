#include "SettingsView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>

SettingsView::SettingsView() : screen(nullptr), contentLabel(nullptr), created(false) {}

SettingsView::~SettingsView() {
    destroy();
}

void SettingsView::create() {
    if (created) return;
    
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    // 禁用主螢幕滾動條
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // 使用OLED布局創建頂部標題欄（單色 OLED 上區域 18px）
    lv_obj_t* topBar = OLEDLayout::createTopBar(screen, "SETTINGS");
    
    // 創建內容區域（單色 OLED 下區域 46px）
    lv_obj_t* contentArea = OLEDLayout::createContentArea(screen);
    
    // 創建設置項目標籤
    contentLabel = lv_label_create(contentArea);
    lv_label_set_text(contentLabel, 
        "WiFi Settings\n"
        "Display Settings\n"
        "System Settings\n"
        "Button Config\n"
        "Screen Timeout\n"
        "Brightness\n"
        "Language\n"
        "Factory Reset");
    lv_obj_set_style_text_color(contentLabel, lv_color_white(), 0);
    
    // 設置字體大小 - 使用 14px 字體改善點陣連續性
    lv_obj_set_style_text_font(contentLabel, &lv_font_montserrat_14, 0);
    
    // 改善文字顯示品質 - 使文字更平滑連續
    lv_obj_set_style_text_opa(contentLabel, LV_OPA_COVER, 0);
    lv_obj_set_style_text_line_space(contentLabel, 0, 0); // 最小行間距
    lv_obj_set_style_text_letter_space(contentLabel, 1, 0); // 微小字母間距改善渲染
    
    // 確保文字對齊和清晰度
    lv_obj_set_style_text_align(contentLabel, LV_TEXT_ALIGN_LEFT, 0);
    
    // 添加文字邊框以改善可讀性
    lv_obj_set_style_outline_width(contentLabel, 0, 0);
    lv_obj_set_style_shadow_width(contentLabel, 0, 0);
    
    lv_obj_set_pos(contentLabel, 2, 2);
    
    created = true;
    Serial.printf("OLED Settings View created with dual-zone layout - screen pointer: %p\n", screen);
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
    Serial.printf("SettingsView::getScreen() - screen pointer: %p, created: %d\n", screen, created);
    return screen;
}

bool SettingsView::isCreated() const {
    return created;
}

void SettingsView::scrollUp() {
    if (!created || !contentLabel) return;
    
    // 獲取當前位置並向上移動
    lv_coord_t currentY = lv_obj_get_y(contentLabel);
    lv_coord_t newY = currentY + 10; // 向上滾動10像素
    
    // 限制滾動範圍（不能滾動超過原始位置）
    if (newY > 2) newY = 2; // 原始位置是 2
    
    lv_obj_set_y(contentLabel, newY);
    Serial.printf("Settings scroll up - Y position: %d\n", newY);
}

void SettingsView::scrollDown() {
    if (!created || !contentLabel) return;
    
    // 獲取當前位置並向下移動
    lv_coord_t currentY = lv_obj_get_y(contentLabel);
    lv_coord_t newY = currentY - 10; // 向下滾動10像素
    
    // 計算滾動範圍以顯示所有 8 個項目
    // 使用14px字體 + 0px行間距 = 14px/行，8項目 = 112px，內容區域46px，需要滾動66px
    if (newY < -70) newY = -70; // 允許滾動到 -70 以顯示所有項目
    
    lv_obj_set_y(contentLabel, newY);
    Serial.printf("Settings scroll down - Y position: %d\n", newY);
}