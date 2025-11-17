#include "MenuView.h"
#include "../../Utils/OLEDLayout.h"
#include "../../Utils/PageManager/PageManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

MenuView::MenuView(MenuModel* m) : model(m), screen(nullptr), created(false), 
    statusBar(nullptr), iconContainer(nullptr), iconHighlight(nullptr), 
    hintArea(nullptr), selectedIcon(0), menuOptionsVisible(false) {
    for(int i = 0; i < 4; i++) {
        iconLabels[i] = nullptr;
        menuLabels[i] = nullptr;
    }
    timeLabel = nullptr;
    dateLabel = nullptr;
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
    
    // 建立 MainMenu 基本三區域：狀態欄 (16px) / 主資訊區 (16px時間顯示) / 提示區 (日期)
    statusBar = OLEDLayout::createStatusBar(screen);

    // 主資訊區 (16px 顯示時間)
    lv_obj_t* mainArea = OLEDLayout::createMainContentArea(screen);
    // 清除原本的圖標容器內容，改為時間大字體顯示
    timeLabel = lv_label_create(mainArea);
    lv_label_set_text(timeLabel, "--:--");
    lv_obj_set_style_text_font(timeLabel, &lv_font_unscii_16, 0);
    lv_obj_set_style_text_color(timeLabel, lv_color_white(), 0);
    lv_obj_center(timeLabel);

    // 底部提示區顯示日期
    hintArea = OLEDLayout::createHintBar(screen, "--- --/--");
    // dateLabel 為 hintArea 的子項（第一個子項）
    dateLabel = lv_obj_get_child(hintArea, 0);

    // 同時建置圖標式選單但初始隱藏（在按 OK 時顯示）
    iconContainer = OLEDLayout::createMainContentArea(screen);
    // 將 iconContainer 視為選單列表容器 (放置垂直列表)
    lv_obj_set_size(iconContainer, 128, 38);
    lv_obj_set_pos(iconContainer, 0, 16);
    lv_obj_set_scrollbar_mode(iconContainer, LV_SCROLLBAR_MODE_OFF);

    // 建立四個簡潔的文字選項 (垂直排列) - 使用高亮顯示而非箭頭
    const char* labels[4] = {"Trekking", "Radio", "System", "Status"};
    for (int i = 0; i < 4; i++) {
        menuLabels[i] = lv_label_create(iconContainer);
        lv_obj_set_width(menuLabels[i], 110);
        lv_label_set_text(menuLabels[i], labels[i]);
        lv_obj_set_style_text_font(menuLabels[i], &lv_font_unscii_8, 0);
        lv_obj_set_style_text_color(menuLabels[i], lv_color_white(), 0);
        lv_obj_set_style_text_align(menuLabels[i], LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_bg_opa(menuLabels[i], LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(menuLabels[i], 0, 0);
        lv_obj_set_pos(menuLabels[i], 8, 1 + i * 8); // 調整位置，不需要為箭頭留空間
    }

    // 初始狀態：顯示時間，隱藏選單
    lv_obj_add_flag(iconContainer, LV_OBJ_FLAG_HIDDEN);
    menuOptionsVisible = false;
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
    // 更新選中的項目使用高亮顯示
    if (!created || index < 0 || index >= 4) return;
    
    // 先清除所有項目的高亮
    for (int i = 0; i < 4; i++) {
        if (menuLabels[i]) {
            lv_obj_set_style_text_color(menuLabels[i], lv_color_white(), 0);
            lv_obj_set_style_bg_opa(menuLabels[i], LV_OPA_TRANSP, 0);
        }
    }
    
    // 設置當前選中項目的高亮
    selectedIcon = index;
    if (menuLabels[selectedIcon]) {
        lv_obj_set_style_text_color(menuLabels[selectedIcon], lv_color_black(), 0);
        lv_obj_set_style_bg_opa(menuLabels[selectedIcon], LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(menuLabels[selectedIcon], lv_color_white(), 0);
    }
    
    Serial.printf("MainMenu updateSelection: %d (highlight mode)\n", index);
}

void MenuView::createIconMenuLayout() {
    // Deprecated: icon-grid layout retained for compatibility but not used by default
}

void MenuView::updateTimeAndDate() {
    // 更新 timeLabel 與 hint(date)
    char timeStr[16], dateStr[16];
    getRealTime(timeStr, dateStr);
    if (timeLabel) lv_label_set_text(timeLabel, timeStr);
    // 時間畫面顯示日期
    if (dateLabel) lv_label_set_text(dateLabel, dateStr);
}

void MenuView::updateBatteryLevel() {
    // 狀態欄更新由 main.cpp 中的 updateStatusBar 統一處理
    // 這裡不再直接更新，避免覆蓋 main.cpp 的時間+電池更新
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

// 顯示圖標選單（進入導航選單模式）
void MenuView::showMenuOptions() {
    if (!created) return;
    lv_obj_clear_flag(iconContainer, LV_OBJ_FLAG_HIDDEN);
    if (timeLabel) lv_obj_add_flag(timeLabel, LV_OBJ_FLAG_HIDDEN);
    // 更新提示為 [OK]
    OLEDLayout::updateHintBar(hintArea, "[OK]");
    menuOptionsVisible = true;
    // 確保高亮顯示與選中項一致
    updateSelection(selectedIcon);
}

// 隱藏圖標選單（回到時間顯示）
void MenuView::hideMenuOptions() {
    if (!created) return;
    lv_obj_add_flag(iconContainer, LV_OBJ_FLAG_HIDDEN);
    if (timeLabel) lv_obj_clear_flag(timeLabel, LV_OBJ_FLAG_HIDDEN);
    // 恢復提示為日期（由 updateTimeAndDate 更新）
    char timeStr[16], dateStr[16];
    getRealTime(timeStr, dateStr);
    OLEDLayout::updateHintBar(hintArea, dateStr);
    menuOptionsVisible = false;
}

bool MenuView::isMenuOptionsVisible() const {
    return menuOptionsVisible;
}

// 狀態欄更新方法
void MenuView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        if (menuOptionsVisible) {
            // 選單模式：顯示時間+電池（跟其他頁面一樣）
            OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
        } else {
            // 時間畫面：只顯示電池
            OLEDLayout::updateStatusBar(statusBar, batteryText, "");
        }
    }
}