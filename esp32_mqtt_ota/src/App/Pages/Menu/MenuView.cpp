#include "MenuView.h"
#include "../../Utils/OLEDLayout.h"
#include "../../Utils/PageManager/PageManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

MenuView::MenuView(MenuModel* m) : model(m), screen(nullptr), created(false), title(nullptr), 
    menuArea(nullptr), batteryLabel(nullptr), timeLabel(nullptr), dateLabel(nullptr) {
    for(int i = 0; i < 4; i++) {
        buttons[i] = nullptr;
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
    
    // 創建MainMenu界面結構
    createMainMenuLayout();
    created = true;
    Serial.println("Menu View created");
}

void MenuView::destroy() {
    if (!created) return;
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
    title = nullptr;
    menuArea = nullptr;
    batteryLabel = nullptr;
    timeLabel = nullptr;
    dateLabel = nullptr;
    for(int i = 0; i < 4; i++) {
        buttons[i] = nullptr;
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
    // MainMenu 不再有選擇功能，此方法保留但不執行任何操作
    if (!created) return;
    Serial.println("MainMenu updateSelection - no action needed");
}

void MenuView::createMainMenuLayout() {
    // 電池容量顯示區域 (頂部 16px)
    lv_obj_t* batteryArea = lv_obj_create(screen);
    lv_obj_set_size(batteryArea, 128, 16);
    lv_obj_set_pos(batteryArea, 0, 0);
    lv_obj_set_style_bg_color(batteryArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(batteryArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(batteryArea, 0, 0);
    lv_obj_set_style_pad_all(batteryArea, 2, 0);
    lv_obj_set_scrollbar_mode(batteryArea, LV_SCROLLBAR_MODE_OFF);
    
    batteryLabel = lv_label_create(batteryArea);
    lv_label_set_text(batteryLabel, "Batt: 85%");
    lv_obj_set_style_text_font(batteryLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(batteryLabel, lv_color_white(), 0);
    lv_obj_set_style_text_align(batteryLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(batteryLabel);
    
            // 主要時鐘顯示區域 (中間 28px)
    lv_obj_t* timeArea = lv_obj_create(screen);
    lv_obj_set_size(timeArea, 128, 28);
    lv_obj_set_pos(timeArea, 0, 16);
    lv_obj_set_style_bg_color(timeArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(timeArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(timeArea, 0, 0);
    lv_obj_set_style_pad_all(timeArea, 2, 0);
    lv_obj_set_scrollbar_mode(timeArea, LV_SCROLLBAR_MODE_OFF);
    
    timeLabel = lv_label_create(timeArea);
    if (timeLabel) {
        lv_label_set_text(timeLabel, "AM 11:59");
        lv_obj_set_style_text_font(timeLabel, &lv_font_unscii_16, 0);
        lv_obj_set_style_text_color(timeLabel, lv_color_white(), 0);
        lv_obj_set_style_text_align(timeLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(timeLabel);
    }
    
    // 日期顯示區域 (底部 10px)
    lv_obj_t* dateArea = lv_obj_create(screen);
    lv_obj_set_size(dateArea, 128, 10);
    lv_obj_set_pos(dateArea, 0, 44);
    lv_obj_set_style_bg_color(dateArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(dateArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(dateArea, 0, 0);
    lv_obj_set_style_pad_all(dateArea, 1, 0);
    lv_obj_set_scrollbar_mode(dateArea, LV_SCROLLBAR_MODE_OFF);
    
    dateLabel = lv_label_create(dateArea);
    lv_obj_set_style_text_font(dateLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(dateLabel, lv_color_white(), 0);
    lv_obj_set_style_text_align(dateLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(dateLabel);
    
    // 更新時間和日期
    updateTimeAndDate();
}

void MenuView::updateTimeAndDate() {
    char timeStr[16];
    char dateStr[16];
    
    // 獲取真實時間
    getRealTime(timeStr, dateStr);
    
    // 更新時間顯示
    if (timeLabel) {
        lv_label_set_text(timeLabel, timeStr);
    }
    
    // 更新日期顯示
    if (dateLabel) {
        lv_label_set_text(dateLabel, dateStr);
    }
}

void MenuView::updateBatteryLevel() {
    if (!batteryLabel) return;
    
    // 讀取電池電壓 (ESP32 ADC)
    // 這裡使用模擬值，實際需要根據硬體連接讀取ADC
    uint32_t voltage = 3700 + (millis() % 500); // 模擬電壓變化 3700-4200mV
    int percentage = map(voltage, 3200, 4200, 0, 100);
    percentage = constrain(percentage, 0, 100);
    
    char battStr[16];
    // Use simpler display format in English
    if (percentage >= 95) {
        snprintf(battStr, sizeof(battStr), "Batt:Full");
    } else if (percentage >= 20) {
        snprintf(battStr, sizeof(battStr), "Batt:%d%%", percentage);
    } else {
        snprintf(battStr, sizeof(battStr), "Batt:Low");
    }
    
    lv_label_set_text(batteryLabel, battStr);
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

void MenuView::button_event_cb(lv_event_t* e) {
    // MainMenu 不再有可點擊的按鈕，此函數保留但不執行任何操作
    Serial.println("MainMenu button event - no action");
}