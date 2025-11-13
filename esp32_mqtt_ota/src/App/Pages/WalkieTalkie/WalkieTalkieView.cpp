#include "WalkieTalkieView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>

WalkieTalkieView::WalkieTalkieView() : 
    screen(nullptr), statusBar(nullptr), contentArea(nullptr), 
    arrowLabel(nullptr), hintBar(nullptr),
    created(false), selectionIndex(0), scrollOffset(0), currentMode(OperationMode::NAVIGATION) {
    // 初始化無線電標籤陣列
    for (int i = 0; i < ITEM_COUNT; i++) {
        radioLabels[i] = nullptr;
    }
    Serial.println("WalkieTalkieView: Constructor");
}

WalkieTalkieView::~WalkieTalkieView() {
    destroy();
    Serial.println("WalkieTalkieView: Destructor");
}

void WalkieTalkieView::create() {
    if (created) {
        Serial.println("WalkieTalkieView: Already created");
        return;
    }
    
    Serial.println("WalkieTalkieView: Creating Radio screen");
    
    // 建立主螢幕
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // 創建統一的狀態欄 (16px 高度)
    statusBar = OLEDLayout::createStatusBar(screen);
    // 設定狀態欄內容（示例固定值，亦可由外部更新）
    OLEDLayout::updateStatusBar(statusBar, "85%", "09:30");

    // 創建中間主資訊區（使用通用工具，38px 高度）
    contentArea = OLEDLayout::createMainContentArea(screen);

    // 創建無線電資訊顯示標籤（ITEM_COUNT 行），arrow 放在左側
    for (int i = 0; i < ITEM_COUNT; i++) {
        radioLabels[i] = lv_label_create(contentArea);
        lv_obj_set_width(radioLabels[i], 110);
        lv_obj_set_style_text_font(radioLabels[i], &lv_font_unscii_8, 0);
        lv_obj_set_style_text_color(radioLabels[i], lv_color_white(), 0);
        lv_obj_set_style_text_align(radioLabels[i], LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_bg_opa(radioLabels[i], LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(radioLabels[i], 0, 0);
        // 初始位置，實際由 updateSelectionDisplay 管理
        lv_obj_set_pos(radioLabels[i], 12, 1 + i * 8);
    }

    // 創建箭頭標籤 (left column)
    arrowLabel = lv_label_create(contentArea);
    lv_label_set_text(arrowLabel, ">");
    lv_obj_set_style_text_font(arrowLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(arrowLabel, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(arrowLabel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(arrowLabel, 0, 0);
    lv_obj_set_pos(arrowLabel, 2, 1);

    // 創建底部功能提示區（10px）
    hintBar = OLEDLayout::createHintBar(screen, "[BACK]");

    // 初始化選取與滾動狀態
    selectionIndex = 0;
    scrollOffset = 0;

    // 初始化顯示內容與選取顯示
    updateDisplay();
    
    // 初始化提示文字
    updateHintText();

    created = true;
    Serial.println("WalkieTalkieView: Created successfully");
}

void WalkieTalkieView::destroy() {
    if (!created) return;
    
    Serial.println("WalkieTalkieView: Destroying");
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
        statusBar = nullptr;
        contentArea = nullptr;
        for (int i = 0; i < ITEM_COUNT; i++) {
            radioLabels[i] = nullptr;
        }
        arrowLabel = nullptr;
        hintBar = nullptr;
    }
    
    created = false;
}

lv_obj_t* WalkieTalkieView::getScreen() const {
    return screen;
}

bool WalkieTalkieView::isCreated() const {
    return created;
}

// 無線電狀態更新
void WalkieTalkieView::updateRadioStatus(const RadioStatus& status) {
    radioStatus = status;
    updateDisplay();
}

// 主要顯示更新方法
void WalkieTalkieView::updateDisplay() {
    if (!created) return;
    
    // 更新六行主資訊內容
    char tmp[64];

    // CH: channel (simple format)
    snprintf(tmp, sizeof(tmp), "CH: %d", radioStatus.channel);
    lv_label_set_text(radioLabels[0], tmp);

    // CTCSS (simplified)
    snprintf(tmp, sizeof(tmp), "CTCSS: %.1f", radioStatus.ctcss);
    lv_label_set_text(radioLabels[1], tmp);

    // Power level
    snprintf(tmp, sizeof(tmp), "Power: %s", radioStatus.powerLevel);
    lv_label_set_text(radioLabels[2], tmp);

    // RSSI (simple format)
    snprintf(tmp, sizeof(tmp), "RSSI: %ddBm", radioStatus.rssi);
    lv_label_set_text(radioLabels[3], tmp);

    // VOL
    snprintf(tmp, sizeof(tmp), "VOL : %d", radioStatus.volumeLevel);
    lv_label_set_text(radioLabels[4], tmp);

    // CQL (squelch)
    snprintf(tmp, sizeof(tmp), "CQL : %d", radioStatus.squelchLevel);
    lv_label_set_text(radioLabels[5], tmp);

    // 更新箭頭位置與滾動顯示
    updateSelectionDisplay();
}

// 頻率格式化輔助函數
void WalkieTalkieView::formatFrequency(float freq, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%.4f", freq);
}

// 更新選取顯示：顯示可見範圍內的行，並將箭頭放到當前選取行
void WalkieTalkieView::updateSelectionDisplay() {
    if (!created || !contentArea) return;

    for (int i = 0; i < ITEM_COUNT; i++) {
        int rel = i - scrollOffset; // 相對於可見區的索引
        if (rel >= 0 && rel < VISIBLE_COUNT) {
            // 可見：計算 y
            int y = 1 + rel * 8; // 每行 8px 間距
            lv_obj_set_pos(radioLabels[i], 12, y);
            lv_obj_clear_flag(radioLabels[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            // 不可見，隱藏
            lv_obj_add_flag(radioLabels[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    // 更新箭頭位置（若選取項在可見範圍）
    int relSel = selectionIndex - scrollOffset;
    if (relSel >= 0 && relSel < VISIBLE_COUNT) {
        int arrowY = 1 + relSel * 8;
        lv_obj_set_pos(arrowLabel, 2, arrowY);
        lv_obj_clear_flag(arrowLabel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(arrowLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

// 靜噪調整
void WalkieTalkieView::adjustSquelch(int delta) {
    radioStatus.squelchLevel += delta;
    if (radioStatus.squelchLevel < 1) radioStatus.squelchLevel = 1;
    if (radioStatus.squelchLevel > 8) radioStatus.squelchLevel = 8;
    Serial.printf("WalkieTalkie: Squelch level: %d\n", radioStatus.squelchLevel);
    updateDisplay();
}

// 音量調整
void WalkieTalkieView::adjustVolume(int delta) {
    radioStatus.volumeLevel += delta;
    if (radioStatus.volumeLevel < 1) radioStatus.volumeLevel = 1;
    if (radioStatus.volumeLevel > 8) radioStatus.volumeLevel = 8;
    Serial.printf("WalkieTalkie: Volume level: %d\n", radioStatus.volumeLevel);
    updateDisplay();
}

// 頻道切換
void WalkieTalkieView::changeChannel(int delta) {
    radioStatus.channel += delta;
    if (radioStatus.channel < 1) radioStatus.channel = 16;
    if (radioStatus.channel > 16) radioStatus.channel = 1;
    
    // 更新對應頻率（簡化的頻率計算）
    radioStatus.frequency = 462.5625f + (radioStatus.channel - 1) * 0.025f;
    
    Serial.printf("WalkieTalkie: Channel: %d, Freq: %.4f MHz\n", 
                  radioStatus.channel, radioStatus.frequency);
    updateDisplay();
}

// 功率切換
void WalkieTalkieView::togglePower() {
    if (strcmp(radioStatus.powerLevel, "LOW") == 0) {
        radioStatus.powerLevel = "MID";
    } else if (strcmp(radioStatus.powerLevel, "MID") == 0) {
        radioStatus.powerLevel = "HIGH";
    } else {
        radioStatus.powerLevel = "LOW";
    }
    Serial.printf("WalkieTalkie: Power level: %s\n", radioStatus.powerLevel);
    updateDisplay();
}

// 按鈕處理方法
void WalkieTalkieView::handleOKButton() {
    // 切換操作模式：NAVIGATION <-> VALUE_EDIT
    switch (currentMode) {
        case OperationMode::NAVIGATION:
            setOperationMode(OperationMode::VALUE_EDIT);
            Serial.println("WalkieTalkieView: Mode -> VALUE_EDIT");
            break;
        case OperationMode::VALUE_EDIT:
            setOperationMode(OperationMode::NAVIGATION);
            Serial.println("WalkieTalkieView: Mode -> NAVIGATION");
            break;
    }
}

void WalkieTalkieView::handleUpButton() {
    switch (currentMode) {
        case OperationMode::NAVIGATION:
            // 導航模式：向上選擇項目
            if (selectionIndex > 0) {
                selectionIndex--;
                if (selectionIndex < scrollOffset) scrollOffset = selectionIndex;
                updateSelectionDisplay();
            }
            Serial.printf("WalkieTalkieView: Navigation UP - Selection: %d\n", selectionIndex);
            break;
        case OperationMode::VALUE_EDIT:
            // 數值編輯模式：增加當前選中項目的數值 (+)
            adjustSelectedValue(1);
            Serial.printf("WalkieTalkieView: Value edit UP (+) - Item: %d\n", selectionIndex);
            break;
    }
}

void WalkieTalkieView::handleDownButton() {
    switch (currentMode) {
        case OperationMode::NAVIGATION:
            // 導航模式：向下選擇項目
            if (selectionIndex < ITEM_COUNT - 1) {
                selectionIndex++;
                if (selectionIndex >= scrollOffset + VISIBLE_COUNT) {
                    scrollOffset = selectionIndex - VISIBLE_COUNT + 1;
                }
                updateSelectionDisplay();
            }
            Serial.printf("WalkieTalkieView: Navigation DOWN - Selection: %d\n", selectionIndex);
            break;
        case OperationMode::VALUE_EDIT:
            // 數值編輯模式：減少當前選中項目的數值 (-)
            adjustSelectedValue(-1);
            Serial.printf("WalkieTalkieView: Value edit DOWN (-) - Item: %d\n", selectionIndex);
            break;
    }
}

void WalkieTalkieView::updateStatusBar(const char* batteryText, const char* timeText) {
    if (statusBar) {
        OLEDLayout::updateStatusBar(statusBar, batteryText, timeText);
    }
}

// 設置操作模式
void WalkieTalkieView::setOperationMode(OperationMode mode) {
    currentMode = mode;
    updateHintText();
}

// 更新提示文字
void WalkieTalkieView::updateHintText() {
    if (!hintBar) return;
    
    const char* hintText;
    switch (currentMode) {
        case OperationMode::NAVIGATION:
            hintText = "[OK]";
            break;
        case OperationMode::VALUE_EDIT:
            hintText = "[BACK]";
            break;
        default:
            hintText = "[OK]";
            break;
    }
    OLEDLayout::updateHintBar(hintBar, hintText);
}

// 調整選中項目的數值
void WalkieTalkieView::adjustSelectedValue(int delta) {
    switch (selectionIndex) {
        case 0: // 頻道
            changeChannel(delta);
            break;
        case 1: // CTCSS (不可調整，跳過)
            break;
        case 2: // 功率 (切換)
            if (delta != 0) togglePower();
            break;
        case 3: // RSSI (只讀，跳過)
            break;
        case 4: // 音量
            adjustVolume(delta);
            break;
        case 5: // 靜噪
            adjustSquelch(delta);
            break;
    }
}