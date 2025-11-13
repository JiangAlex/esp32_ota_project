#include "TrekkingView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>

TrekkingView::TrekkingView() : 
    screen(nullptr), statusBar(nullptr), contentArea(nullptr), 
    created(false), currentState(TrekkingState::START), startTime(0) {
    // 初始化數據標籤陣列
    for (int i = 0; i < 4; i++) {
        dataLabels[i] = nullptr;
    }
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
    
    Serial.println("TrekkingView: Creating Trekking screen");
    
    // 建立主螢幕
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // 1. 創建頂部狀態欄（16px）：電池 + 時間
    statusBar = OLEDLayout::createStatusBar(screen);
    
    // 2. 創建中間主資訊區（38px）：Trekking數據
    contentArea = OLEDLayout::createMainContentArea(screen);
    
    // 創建數據顯示標籤（調整為適應38px高度，4行每行約9px）
    for (int i = 0; i < 4; i++) {
        dataLabels[i] = lv_label_create(contentArea);
        lv_obj_set_width(dataLabels[i], 124);  // 稍微縮小寬度
        lv_obj_set_style_text_font(dataLabels[i], &lv_font_unscii_8, 0);
        lv_obj_set_style_text_color(dataLabels[i], lv_color_white(), 0);
        lv_obj_set_style_text_align(dataLabels[i], LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_bg_opa(dataLabels[i], LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(dataLabels[i], 0, 0);
        lv_obj_set_pos(dataLabels[i], 2, 1 + i * 9);  // 調整為9px間距以適應38px高度
    }
    
    // 3. 創建底部功能提示區（10px）：操作提示
    hintBar = OLEDLayout::createHintBar(screen, "[OK] START [BACK] Exit");
    
    // 設置初始狀態為 START
    setState(TrekkingState::START);
    
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
        statusBar = nullptr;
        contentArea = nullptr;
        hintBar = nullptr;
        for (int i = 0; i < 4; i++) {
            dataLabels[i] = nullptr;
        }
    }
    
    created = false;
}

lv_obj_t* TrekkingView::getScreen() const {
    return screen;
}

bool TrekkingView::isCreated() const {
    return created;
}

// 狀態管理方法
void TrekkingView::setState(TrekkingState state) {
    currentState = state;
    if (currentState == TrekkingState::RUNNING && startTime == 0) {
        startTime = millis();  // 記錄開始時間
    }
    updateDisplay();
}

// 環境數據更新
void TrekkingView::updateEnvironmentData(float temp, float alt, float press) {
    trekkingData.temperature = temp;
    trekkingData.altitude = alt;
    trekkingData.pressure = press;
    updateDisplay();
}

// 累計數據更新
void TrekkingView::updateCumulativeData(unsigned long elapsed, float dist, float asc, int steps) {
    trekkingData.elapsedTime = elapsed;
    trekkingData.distance = dist;
    trekkingData.ascent = asc;
    trekkingData.stepCount = steps;
    updateDisplay();
}

// 時間格式化輔助函數
void TrekkingView::formatTime(unsigned long seconds, char* buffer, size_t bufferSize) {
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    snprintf(buffer, bufferSize, "%02lu:%02lu:%02lu", hours, minutes, secs);
}

// 主要顯示更新方法
void TrekkingView::updateDisplay() {
    if (!created) return;
    
    switch (currentState) {
        case TrekkingState::START:
            createStartLayout();
            break;
        case TrekkingState::RUNNING:
            createRunningLayout();
            break;
        case TrekkingState::PAUSED:
            createRunningLayout();  // 暫停狀態顯示與運行相同，只是按鈕不同
            break;
    }
}

// START 狀態佈局
void TrekkingView::createStartLayout() {
    if (!created) return;
    
    // 第一行：標題
    lv_label_set_text(dataLabels[0], "--- START TREK ---");
    
    // 第二行：溫度
    char tempStr[32];
    snprintf(tempStr, sizeof(tempStr), "T: %.1f°C", trekkingData.temperature);
    lv_label_set_text(dataLabels[1], tempStr);
    
    // 第三行：海拔和氣壓
    char altPressStr[32];
    snprintf(altPressStr, sizeof(altPressStr), "ALT: %.0fm P: %.0fhPa", 
             trekkingData.altitude, trekkingData.pressure);
    lv_label_set_text(dataLabels[2], altPressStr);
    
    // 第四行：狀態資訊
    lv_label_set_text(dataLabels[3], "Ready to start...");
    
    // 更新底部提示區
    OLEDLayout::updateHintBar(hintBar, "[OK] START [BACK] Exit");
}

// RUNNING 狀態佈局
void TrekkingView::createRunningLayout() {
    if (!created) return;
    
    // 第一行：時間和距離
    char timeDistStr[32];
    char timeStr[16];
    formatTime(trekkingData.elapsedTime, timeStr, sizeof(timeStr));
    snprintf(timeDistStr, sizeof(timeDistStr), "%s <-> %.1fKM", timeStr, trekkingData.distance);
    lv_label_set_text(dataLabels[0], timeDistStr);
    
    // 第二行：海拔、溫度、氣壓
    char envStr[32];
    snprintf(envStr, sizeof(envStr), "%.0fm %.1f°C %.0fhPa", 
             trekkingData.altitude, trekkingData.temperature, trekkingData.pressure);
    lv_label_set_text(dataLabels[1], envStr);
    
    // 第三行：爬升和步數
    char ascentStepStr[32];
    snprintf(ascentStepStr, sizeof(ascentStepStr), "ASC: %.0fm STEP: %d", 
             trekkingData.ascent, trekkingData.stepCount);
    lv_label_set_text(dataLabels[2], ascentStepStr);
    
    // 第四行：狀態資訊 
    if (currentState == TrekkingState::RUNNING) {
        lv_label_set_text(dataLabels[3], "TREKKING IN PROGRESS");
    } else {
        lv_label_set_text(dataLabels[3], "TREKKING PAUSED");
    }
    
    // 更新底部提示區
    if (currentState == TrekkingState::RUNNING) {
        OLEDLayout::updateHintBar(hintBar, "[OK] PAUSE [BACK] Stop");
    } else {
        OLEDLayout::updateHintBar(hintBar, "[OK] RESUME [BACK] Stop");
    }
}

// 按鈕處理方法
void TrekkingView::handleOKButton() {
    switch (currentState) {
        case TrekkingState::START:
            setState(TrekkingState::RUNNING);
            Serial.println("TrekkingView: Started tracking");
            break;
        case TrekkingState::RUNNING:
            setState(TrekkingState::PAUSED);
            Serial.println("TrekkingView: Paused tracking");
            break;
        case TrekkingState::PAUSED:
            setState(TrekkingState::RUNNING);
            Serial.println("TrekkingView: Resumed tracking");
            break;
    }
}

void TrekkingView::handleUpButton() {
    // 在 START 狀態下，UP 按鈕可用於返回主選單
    if (currentState == TrekkingState::START) {
        Serial.println("TrekkingView: UP pressed - back to main menu");
    }
    // 在 RUNNING 狀態下可以用來調整顯示或其他功能
}

void TrekkingView::handleDownButton() {
    // 在 RUNNING 狀態下，DOWN 按鈕可用於停止並重置
    if (currentState == TrekkingState::RUNNING || currentState == TrekkingState::PAUSED) {
        setState(TrekkingState::START);
        // 重置數據
        trekkingData.elapsedTime = 0;
        trekkingData.distance = 0;
        trekkingData.ascent = 0;
        trekkingData.stepCount = 0;
        startTime = 0;
        Serial.println("TrekkingView: Reset to START state");
    }
}

void TrekkingView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
    }
}