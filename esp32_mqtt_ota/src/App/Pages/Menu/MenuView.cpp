#include "MenuView.h"
#include "../../Utils/OLEDLayout.h"
#include "../../Utils/PageManager/PageManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

MenuView::MenuView(MenuModel* m) : model(m), screen(nullptr), created(false), 
    statusBar(nullptr), iconContainer(nullptr), iconHighlight(nullptr), 
    hintArea(nullptr), selectedIcon(0) {
    for(int i = 0; i < 4; i++) {
        iconLabels[i] = nullptr;
    }
}

MenuView::~MenuView() {
    destroy();
}

void MenuView::create() {
    if (created) return;
    
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    // 禁用主螢幕滾動條
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // 初始化RTC
    initRTC();
    
    // 創建圖標式MainMenu界面
    createIconMenuLayout();
    created = true;
    Serial.println("Menu View (Icon Mode) created");
}

void MenuView::destroy() {
    if (!created) return;
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
    statusBar = nullptr;
    iconContainer = nullptr;
    iconHighlight = nullptr;
    hintArea = nullptr;
    for(int i = 0; i < 4; i++) {
        iconLabels[i] = nullptr;
    }
    created = false;
    Serial.println("MainMenu View destroyed");
}

lv_obj_t* MenuView::getScreen() const {
    return screen;
}

bool MenuView::isCreated() const {
    return created;
}

void MenuView::updateSelection(int index) {
    // 更新選中的圖標
    if (!created || index < 0 || index >= 4) return;
    selectedIcon = index;
    updateIconHighlight();
    Serial.printf("MainMenu updateSelection: %d\n", index);
}

void MenuView::createIconMenuLayout() {
    // 1. 創建頂部狀態欄（16px）：電池 + 時間
    statusBar = OLEDLayout::createStatusBar(screen);
    
    // 2. 創建中間主資訊區（38px）：圖標選單
    iconContainer = OLEDLayout::createMainContentArea(screen);
    
    // 創建 2x2 圖標按鈕（適應38px高度）
    // 第一行
    createIconButton(0, 0, 0, "Trk", "Trekking");  // ⛰️ 替代符號
    createIconButton(1, 0, 1, "Rad", "Radio");     // 🎙️ 替代符號
    
    // 第二行
    createIconButton(2, 1, 0, "Sys", "System");    // ⚙️ 替代符號
    createIconButton(3, 1, 1, "Sta", "Status");    // 📶 替代符號
    
    // 創建選中高亮框
    iconHighlight = lv_obj_create(iconContainer);
    lv_obj_set_size(iconHighlight, 30, 17);  // 適應新高度
    lv_obj_set_style_bg_opa(iconHighlight, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(iconHighlight, lv_color_white(), 0);
    lv_obj_set_style_border_width(iconHighlight, 1, 0);
    lv_obj_set_scrollbar_mode(iconHighlight, LV_SCROLLBAR_MODE_OFF);
    
    // 3. 創建底部功能提示區（10px）：操作提示
    hintArea = OLEDLayout::createHintBar(screen, "[UP/DN] Option [OK] Confirm");
    
    // 設置初始選中項目並更新高亮
    selectedIcon = 0;
    updateIconHighlight();
}

void MenuView::updateTimeAndDate() {
    // 新的圖標式界面使用統一的狀態欄來顯示時間
    // 這個方法現在主要用於 main.cpp 中的定期更新調用
    // 實際的時間顯示通過 updateStatusBar() 方法處理
}

void MenuView::updateBatteryLevel() {
    // 新的圖標式界面使用統一的狀態欄來顯示電池狀態
    // 這個方法現在主要用於 main.cpp 中的定期更新調用
    // 實際的電池顯示通過 updateStatusBar() 方法處理
}

void MenuView::initRTC() {
    // 配置台灣時區 (UTC+8, 無夏令時間)
    // 使用台灣和亞洲的NTP伺服器以獲得更準確的時間同步
    configTime(8 * 3600, 0, "tw.pool.ntp.org", "asia.pool.ntp.org", "pool.ntp.org");
    
    // 設定初始台灣時間 (如果NTP不可用)
    struct tm timeinfo;
    timeinfo.tm_year = 2025 - 1900;  // 2025年
    timeinfo.tm_mon = 10;            // 11月 (0-11，10代表11月)
    timeinfo.tm_mday = 11;           // 11日
    timeinfo.tm_hour = 14;           // 下午2時 (24小時制)
    timeinfo.tm_min = 30;            // 30分
    timeinfo.tm_sec = 0;             // 0秒
    timeinfo.tm_isdst = 0;           // 台灣無夏令時間
    
    time_t t = mktime(&timeinfo);
    struct timeval tv = { .tv_sec = t };
    settimeofday(&tv, nullptr);
    
    Serial.println("RTC initialized - Taiwan Time Zone (UTC+8)");
    Serial.println("NTP servers: tw.pool.ntp.org, asia.pool.ntp.org");
}

void MenuView::getRealTime(char* timeStr, char* dateStr) {
    struct tm timeinfo;
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);
    
    // 檢查時間是否有效 (確保年份在合理範圍內)
    if (timeinfo.tm_year < 125 || timeinfo.tm_year > 200) {  // 2025-2100年範圍
        // 時間無效，使用預設台灣時間
        strcpy(timeStr, "14:30");
        strcpy(dateStr, "MON 11/11");
        Serial.println("Using default time - NTP sync may be pending");
        return;
    }
    
    // 格式化時間 (24小時制，台灣習慣)
    strftime(timeStr, 16, "%H:%M", &timeinfo);
    
    // Format date (English weekday abbreviation + month/day format)
    const char* weekdays_en[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    snprintf(dateStr, 16, "%s %02d/%02d", 
             weekdays_en[timeinfo.tm_wday], 
             timeinfo.tm_mon + 1, 
             timeinfo.tm_mday);
    
    // 輸出偵錯資訊
    Serial.printf("Taiwan Time: %s, Date: %s\n", timeStr, dateStr);
}

// button_event_cb method removed - no longer needed for icon-based menu

// 創建單個圖標按鈕（適應38px高度的主資訊區）
void MenuView::createIconButton(int index, int row, int col, const char* iconText, const char* labelText) {
    if (index >= 4) return;
    
    // 計算位置 (2x2 網格，適應38px高度，每個格子約 32x17)
    int x = col * 32 + 2;
    int y = row * 17 + 2;  // 調整為17px高度以適應38px總高度
    
    // 創建圖標標籤
    iconLabels[index] = lv_label_create(iconContainer);
    lv_obj_set_size(iconLabels[index], 28, 15);  // 增加高度到15px
    lv_obj_set_pos(iconLabels[index], x, y);
    
    // 設置圖標文字（由於不能顯示emoji，使用縮寫）
    lv_label_set_text_fmt(iconLabels[index], "%s\n%s", iconText, labelText);
    lv_obj_set_style_text_font(iconLabels[index], &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(iconLabels[index], lv_color_white(), 0);
    lv_obj_set_style_text_align(iconLabels[index], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(iconLabels[index], LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(iconLabels[index], 0, 0);
}

// 圖標選擇方法
void MenuView::selectNextIcon() {
    selectedIcon = (selectedIcon + 1) % 4;
    updateIconHighlight();
    Serial.printf("Selected icon: %d\n", selectedIcon);
}

void MenuView::selectPrevIcon() {
    selectedIcon = (selectedIcon - 1 + 4) % 4;
    updateIconHighlight();
    Serial.printf("Selected icon: %d\n", selectedIcon);
}

void MenuView::confirmSelection() {
    MenuIcon selected = getSelectedIcon();
    Serial.printf("Confirmed selection: %d\n", static_cast<int>(selected));
    
    // 這裡將在 main.cpp 中處理頁面切換
    // 不在 View 層直接處理頁面邏輯
}

// 更新選中高亮框位置
void MenuView::updateIconHighlight() {
    if (!iconHighlight) return;
    
    int x, y;
    getIconPosition(selectedIcon, &x, &y);
    lv_obj_set_pos(iconHighlight, x, y);
}

// 計算圖標位置（適應38px高度）
void MenuView::getIconPosition(int index, int* x, int* y) {
    int row = index / 2;
    int col = index % 2;
    *x = col * 32 + 1;  // 32px 寬度，1px 偏移
    *y = row * 17 + 1;  // 17px 高度，1px 偏移（適應38px總高度）
}

// 狀態欄更新方法
void MenuView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
    }
}