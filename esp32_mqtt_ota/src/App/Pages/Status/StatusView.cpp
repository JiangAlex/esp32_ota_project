#include "StatusView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>

StatusView::StatusView() : screen(nullptr), created(false), statusLabel(nullptr), statusBar(nullptr) {}

StatusView::~StatusView() {
    destroy();
}

void StatusView::create() {
    if (created) return;
    
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    // 禁用主螢幕滾動條
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
    
    // 創建狀態信息標籤
    statusLabel = lv_label_create(contentArea);
    
    // 設置標籤寬度和自動換行
    lv_obj_set_width(statusLabel, 120); // 設置寬度 (128-8px邊距)
    lv_label_set_long_mode(statusLabel, LV_LABEL_LONG_WRAP); // 啟用自動換行
    lv_obj_set_style_text_line_space(statusLabel, 1, 0); // 增加行間距以提高可讀性
    
    updateSystemStatus(); // 初始化狀態信息
    lv_obj_set_style_text_color(statusLabel, lv_color_white(), 0);
    
    // 設置字體大小 - 使用 UNSCII 8px 點陣字體專為OLED優化
    lv_obj_set_style_text_font(statusLabel, &lv_font_unscii_8, 0);
    
    // 改善文字顯示品質 - 針對緊湊顯示優化
    lv_obj_set_style_text_opa(statusLabel, LV_OPA_COVER, 0);
    lv_obj_set_style_text_line_space(statusLabel, 0, 0); // 最小行間距
    lv_obj_set_style_text_letter_space(statusLabel, 0, 0); // 點陣字體無需字母間距
    
    // 確保文字對齊和清晰度
    lv_obj_set_style_text_align(statusLabel, LV_TEXT_ALIGN_LEFT, 0);
    
    // 添加文字邊框以改善可讀性
    lv_obj_set_style_outline_width(statusLabel, 0, 0);
    lv_obj_set_style_shadow_width(statusLabel, 0, 0);
    
    lv_obj_set_pos(statusLabel, 2, 2);
    
    created = true;
    Serial.println("OLED Status View created with dual-zone layout");
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

void StatusView::updateSystemStatus() {
    if (!created || !statusLabel) return;
    
    // 獲取系統運行時間
    unsigned long uptimeMs = millis();
    unsigned long uptimeSeconds = uptimeMs / 1000;
    unsigned long hours = uptimeSeconds / 3600;
    unsigned long minutes = (uptimeSeconds % 3600) / 60;
    unsigned long seconds = uptimeSeconds % 60;
    
    // 獲取可用記憶體（ESP32）
    size_t freeHeap = ESP.getFreeHeap();
    size_t totalHeap = ESP.getHeapSize();
    
    // 格式化狀態信息（優化為自動換行佈局）
    char statusText[250];
    snprintf(statusText, sizeof(statusText),
        "System Status: OK\n"
        "Memory: %dK / %dK free\n"  
        "Uptime: %02lu:%02lu:%02lu\n"
        "GPIO Status:\n"
        "32:OK 33:OK 34:-- WiFi:--",
        (int)(freeHeap/1024), (int)(totalHeap/1024),
        hours, minutes, seconds
    );
    
    lv_label_set_text(statusLabel, statusText);
    Serial.printf("Status updated - Uptime: %02lu:%02lu:%02lu, Free Memory: %dKB\n", 
                  hours, minutes, seconds, (int)(freeHeap/1024));
}

void StatusView::scrollUp() {
    if (!created || !statusLabel) return;
    
    // 獲取當前位置並向上移動
    lv_coord_t currentY = lv_obj_get_y(statusLabel);
    lv_coord_t newY = currentY + 5; // 向上滾動5像素
    
    // 限制滾動範圍（不能滾動超過原始位置）
    if (newY > 2) newY = 2;
    
    lv_obj_set_y(statusLabel, newY);
    Serial.printf("Status scroll up - Y position: %d\n", newY);
}

void StatusView::scrollDown() {
    if (!created || !statusLabel) return;
    
    // 獲取當前位置並向下移動
    lv_coord_t currentY = lv_obj_get_y(statusLabel);
    lv_coord_t newY = currentY - 5; // 向下滾動5像素
    
    // 擴大滾動範圍以顯示所有狀態信息（包括 GPIO 狀態）
    // 4行文字 × 14px/行 = 56px 總高度，內容區域46px，需要滾動約20px
    if (newY < -30) newY = -30; // 允許滾動到 -30 以顯示所有內容
    
    lv_obj_set_y(statusLabel, newY);
    Serial.printf("Status scroll down - Y position: %d\n", newY);
}

void StatusView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
    }
}