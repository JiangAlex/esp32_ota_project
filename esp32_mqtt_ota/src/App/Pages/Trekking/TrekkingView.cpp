#include "TrekkingView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>

TrekkingView::TrekkingView() : 
    screen(nullptr), statusBar(nullptr), contentArea(nullptr), 
    created(false), currentState(TrekkingState::START), 
    startTime(0), downKeyPressTime(0), downKeyPressed(false) {
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
    
    // 2. 創建內容區域（38px 高度，參考Status頁面）
    contentArea = OLEDLayout::createMainContentArea(screen);
    
    // 3. 創建底部提示區
    lv_obj_t* hintArea = OLEDLayout::createHintBar(screen, "[BACK]");
    
    // 創建單一標籤用於顯示所有Trek信息（參考Status頁面）
    dataLabels[0] = lv_label_create(contentArea);
    
    // 設置標籤寬度和自動換行（參考Status頁面）
    lv_obj_set_width(dataLabels[0], 120); // 設置寬度 (128-8px邊距)
    lv_label_set_long_mode(dataLabels[0], LV_LABEL_LONG_WRAP); // 啟用自動換行
    lv_obj_set_style_text_color(dataLabels[0], lv_color_white(), 0);
    
    // 設置字體大小 - 使用 UNSCII 8px 點陣字體（參考Status頁面）
    lv_obj_set_style_text_font(dataLabels[0], &lv_font_unscii_8, 0);
    
    // 改善文字顯示品質 - 針對緊湊顯示優化（參考Status頁面）
    lv_obj_set_style_text_opa(dataLabels[0], LV_OPA_COVER, 0);
    lv_obj_set_style_text_line_space(dataLabels[0], 0, 0); // 最小行間距
    lv_obj_set_style_text_letter_space(dataLabels[0], 0, 0); // 點陣字體無需字母間距
    
    // 確保文字對齊和清晰度（參考Status頁面）
    lv_obj_set_style_text_align(dataLabels[0], LV_TEXT_ALIGN_LEFT, 0);
    
    // 添加文字邊框以改善可讀性（參考Status頁面）
    lv_obj_set_style_outline_width(dataLabels[0], 0, 0);
    lv_obj_set_style_shadow_width(dataLabels[0], 0, 0);
    
    lv_obj_set_pos(dataLabels[0], 2, 2);
    
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
    
    // 檢查DOWN鍵長按 (在RUNNING或PAUSED狀態下)
    checkDownKeyLongPress();
    
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
        case TrekkingState::END:
            createEndLayout();
            break;
    }
}

// START 狀態佈局（參考Status頁面的單一標籤模式）
void TrekkingView::createStartLayout() {
    if (!created || !dataLabels[0]) return;
    
    // 格式化顯示文本（參考Status頁面）
    char displayText[200];
    snprintf(displayText, sizeof(displayText),
        "T: %.1fC\n"           // 溫度
        "H: %.0f m\n"          // 海拔  
        "P: %.0f hPa\n"        // 氣壓
        "STATUS: START"        // 狀態
    ,
    trekkingData.temperature,
    trekkingData.altitude,
    trekkingData.pressure
    );
    
    lv_label_set_text(dataLabels[0], displayText);
    Serial.printf("START Layout - T:%.1f°C, H:%.0fm, P:%.0fhPa\n", 
                  trekkingData.temperature, trekkingData.altitude, trekkingData.pressure);
}

// RUNNING/PAUSED 狀態佈局（參考Status頁面的單一標籤模式）
void TrekkingView::createRunningLayout() {
    if (!created || !dataLabels[0]) return;
    
    // 格式化時間
    char timeStr[16];
    formatTime(trekkingData.elapsedTime, timeStr, sizeof(timeStr));
    
    // 格式化顯示文本（參考Status頁面）
    char displayText[200];
    const char* statusText = (currentState == TrekkingState::RUNNING) ? "RUNNING" : "PAUSED";
    
    snprintf(displayText, sizeof(displayText),
        "TIME: %s\n"           // 經過時間
        "DIST: %.1f km\n"      // 累計距離
        "ASC: %.0f m\n"        // 累計爬升
        "STEP: %d\n"           // 步數
        "STATUS: %s"           // 狀態
    ,
    timeStr,
    trekkingData.distance,
    trekkingData.ascent,
    trekkingData.stepCount,
    statusText
    );
    
    lv_label_set_text(dataLabels[0], displayText);
    Serial.printf("TREK Layout - Time:%s, Dist:%.1fkm, ASC:%.0fm, Steps:%d, Status:%s\n", 
                  timeStr, trekkingData.distance, trekkingData.ascent, trekkingData.stepCount, statusText);
}

// 按鈕處理方法 - OK按鍵用於狀態循環
void TrekkingView::handleOKButton() {
    // 狀態循環邏輯：START→RUNNING→PAUSED→RUNNING（循環）
    switch (currentState) {
        case TrekkingState::START:
            setState(TrekkingState::RUNNING);
            Serial.println("TrekkingView: State changed from START to RUNNING");
            break;
        case TrekkingState::RUNNING:
            setState(TrekkingState::PAUSED);
            Serial.println("TrekkingView: State changed from RUNNING to PAUSED");
            break;
        case TrekkingState::PAUSED:
            setState(TrekkingState::RUNNING);
            Serial.println("TrekkingView: State changed from PAUSED to RUNNING");
            break;
        case TrekkingState::END:
            // END狀態下不處理OK按鍵
            Serial.println("TrekkingView: In END state, OK button ignored");
            break;
    }
}

void TrekkingView::handleUpButton() {
    // UP 按鍵用於向上滾動（參考Status頁面）
    scrollUp();
}

void TrekkingView::handleDownButton() {
    // DOWN 按鍵用於向下滾動（參考Status頁面）
    scrollDown();
}

// DOWN鍵按下處理
void TrekkingView::handleDownButtonPress() {
    if (currentState == TrekkingState::RUNNING || currentState == TrekkingState::PAUSED) {
        downKeyPressed = true;
        downKeyPressTime = millis();
        Serial.println("TrekkingView: DOWN key pressed, starting long press detection");
    }
}

// DOWN鍵釋放處理
void TrekkingView::handleDownButtonRelease() {
    if (downKeyPressed) {
        unsigned long pressDuration = millis() - downKeyPressTime;
        downKeyPressed = false;
        
        if (pressDuration < 2000) {
            Serial.printf("TrekkingView: DOWN key released after %lu ms (short press)\n", pressDuration);
        } else {
            Serial.printf("TrekkingView: DOWN key released after %lu ms (long press detected)\n", pressDuration);
        }
    }
}

// 檢查DOWN鍵長按 - 需要在主循環中調用
void TrekkingView::checkDownKeyLongPress() {
    if (downKeyPressed && (currentState == TrekkingState::RUNNING || currentState == TrekkingState::PAUSED)) {
        unsigned long pressDuration = millis() - downKeyPressTime;
        if (pressDuration >= 2000) { // 2秒長按
            // 長按DOWN鍵2秒，進入END狀態
            setState(TrekkingState::END);
            downKeyPressed = false; // 重置按鍵狀態
            Serial.println("TrekkingView: Long press detected (2s) - Entering END state");
        }
    }
}

// END 狀態佈局（參考Status頁面的單一標籤模式）
void TrekkingView::createEndLayout() {
    if (!created || !dataLabels[0]) return;
    
    // 格式化總時間
    char timeStr[16];
    formatTime(trekkingData.elapsedTime, timeStr, sizeof(timeStr));
    
    // 格式化顯示文本（參考Status頁面）
    char displayText[200];
    snprintf(displayText, sizeof(displayText),
        "TOTAL: %s\n"          // 總時間
        "DIST: %.1f km\n"      // 總距離
        "ASC: %.0f m\n"        // 總爬升
        "STEP: %d\n"           // 總步數
        "STATUS: END"          // 狀態
    ,
    timeStr,
    trekkingData.distance,
    trekkingData.ascent,
    trekkingData.stepCount
    );
    
    lv_label_set_text(dataLabels[0], displayText);
    Serial.printf("END Layout - Total:%s, Dist:%.1fkm, ASC:%.0fm, Steps:%d\n", 
                  timeStr, trekkingData.distance, trekkingData.ascent, trekkingData.stepCount);
}

// 滾動方法（參考Status頁面）
void TrekkingView::scrollUp() {
    if (!created || !dataLabels[0]) return;
    
    // 獲取當前位置並向上移動
    lv_coord_t currentY = lv_obj_get_y(dataLabels[0]);
    lv_coord_t newY = currentY + 5; // 向上滾動5像素
    
    // 限制滾動範圍（不能滾動超過原始位置）
    if (newY > 2) newY = 2;
    
    lv_obj_set_y(dataLabels[0], newY);
    Serial.printf("Trek scroll up - Y position: %d\n", newY);
}

void TrekkingView::scrollDown() {
    if (!created || !dataLabels[0]) return;
    
    // 獲取當前位置並向下移動
    lv_coord_t currentY = lv_obj_get_y(dataLabels[0]);
    lv_coord_t newY = currentY - 5; // 向下滾動5像素
    
    // 限制滾動範圍以顯示所有Trek信息
    // 5行文字，需要足夠的滾動空間
    if (newY < -20) newY = -20; // 允許滾動到 -20 以顯示所有內容
    
    lv_obj_set_y(dataLabels[0], newY);
    Serial.printf("Trek scroll down - Y position: %d\n", newY);
}

void TrekkingView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
    }
}