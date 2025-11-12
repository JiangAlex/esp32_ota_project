#include "StatusView.h"
#include "../../Utils/OLEDLayout.h"
#include <Arduino.h>
#include <math.h>

StatusView::StatusView() : screen(nullptr), created(false), statusLabel(nullptr), statusBar(nullptr), 
    sensorsAccount(nullptr), stepCounterAccount(nullptr), sensorsHardwareAvailable(false), lastSensorCheck(0) {}

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
    
    // 設置字體大小 - 使用 UNSCII 8px 點陣字體
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
    
    // 安全的 GY-80 傳感器初始化與診斷
    Serial.println("=== GY-80 Sensor Initialization & Diagnosis ===");
    
    bool i2cInitialized = false;
    bool sensorsAvailable = false;
    int deviceCount = 0;
    
    // 第一步：安全的 I2C 初始化
    Serial.println("Step 1: Initializing I2C bus...");
    
    try {
        Wire.begin(21, 22, 100000); // 使用低頻率確保穩定性
        delay(100);
        i2cInitialized = true;
        Serial.println("✓ I2C bus initialized successfully (SDA=21, SCL=22, 100kHz)");
    } catch (...) {
        Serial.println("✗ I2C initialization failed - using simulated data");
        i2cInitialized = false;
    }
    
    // 第二步：設備掃描 (只有在 I2C 初始化成功後)
    if (i2cInitialized) {
        Serial.println("Step 2: Scanning for I2C devices...");
        
        for (uint8_t addr = 0x1E; addr <= 0x77; addr++) {
            if (addr == 0x1E || addr == 0x53 || addr == 0x68 || addr == 0x77) {
                Wire.beginTransmission(addr);
                if (Wire.endTransmission() == 0) {
                    deviceCount++;
                    Serial.printf("✓ Found device at 0x%02X", addr);
                    
                    switch (addr) {
                        case 0x1E: Serial.print(" (HMC5883L Magnetometer)"); sensorsAvailable = true; break;
                        case 0x53: Serial.print(" (ADXL345 Accelerometer)"); sensorsAvailable = true; break;
                        case 0x68: Serial.print(" (ITG3200 Gyroscope)"); break;
                        case 0x77: Serial.print(" (BMP180 Pressure/Temperature)"); sensorsAvailable = true; break;
                    }
                    Serial.println();
                }
            }
        }
        
        Serial.printf("Found %d GY-80 related devices\n", deviceCount);
    }
    
    // 第三步：顯示狀態和建議
    if (sensorsAvailable) {
        Serial.println("✓ GY-80 sensors detected - attempting initialization...");
        
        // 嘗試初始化 GY-80 庫
        if (gy80.begin(Wire, 21, 22, 100000)) {
            Serial.println("✓ GY-80 library initialized successfully");
        } else {
            Serial.println("⚠ GY-80 library init failed, but devices detected");
            sensorsAvailable = false; // 回退到模擬模式
        }
    } else {
        Serial.println("✗ No GY-80 sensors found");
        Serial.println();
        Serial.println("Hardware troubleshooting checklist:");
        Serial.println("1. Power: GY-80 VCC -> ESP32 3.3V, GND -> GND");
        Serial.println("2. Connections: SDA -> GPIO21, SCL -> GPIO22");
        Serial.println("3. Pull-ups: Add 4.7kΩ resistors from SDA/SCL to 3.3V");
        Serial.println("4. Wiring: Check for loose connections or shorts");
        Serial.println("5. Module: Verify GY-80 module is not damaged");
    }
    
    Serial.printf("Sensor mode: %s\n", sensorsAvailable ? "HARDWARE" : "SIMULATED");
    
    // 保存傳感器狀態
    sensorsHardwareAvailable = sensorsAvailable;
    lastSensorCheck = millis();
    
    // 獲取 DataProc 系統中的傳感器帳號
    DataCenter* dataCenter = DataProc::Center();
    sensorsAccount = dataCenter->SearchAccount("Sensors");
    stepCounterAccount = dataCenter->SearchAccount("StepCounter");
    
    if (sensorsAccount) {
        Serial.println("StatusView: Connected to Sensors DataProc account");
    }
    
    if (stepCounterAccount) {
        Serial.println("StatusView: Connected to StepCounter DataProc account");
        // 初始化計步器數據
        DataProc::StepCounter_Info_t stepData = {0, 0, 0.0f, false};
        stepCounterAccount->Commit(&stepData, sizeof(stepData));
        stepCounterAccount->Publish();
    }
    
    created = true;
    Serial.println("OLED Status View created with DataProc sensor support");
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
    updateSensorStatus();
}

void StatusView::updateSensorStatus() {
    if (!created || !statusLabel) return;
    
    float temperature, pressure, altitude, heading;
    const char* compassDir;
    uint32_t stepCount = 0;
    bool dataFromHardware = false;
    
    // 定期重新檢查傳感器狀態 (每30秒)
    uint32_t currentTime = millis();
    if (!sensorsHardwareAvailable && (currentTime - lastSensorCheck > 30000)) {
        recheckSensorHardware();
    }
    
    // 嘗試從硬體讀取數據
    if (sensorsHardwareAvailable) {
        GY80::BMPData bmpData = gy80.readBMP();
        GY80::MagData magData = gy80.readMag();
        GY80::AccelData accelData = gy80.readAccel();
        
        // 檢查讀取是否成功
        if (bmpData.ok || magData.ok || accelData.ok) {
            // 使用實際硬體數據
            temperature = bmpData.ok ? (bmpData.temperature - 13.0f) : 25.5f;
            pressure = bmpData.ok ? (bmpData.pressure) : 1012.0f;
            altitude = calculateAltitude(pressure, 1015.0f) + 40.0f; // 海拔校正 +70m
            heading = magData.ok ? calculateHeading(magData.mx, magData.my) : 270.0f;
            
            // 磁偏角校正
            heading += -4.0f;
            if (heading < 0) heading += 360.0f;
            if (heading >= 360.0f) heading -= 360.0f;
            compassDir = getCompassDirection(heading);
            
            dataFromHardware = true;
            Serial.printf("Hardware sensor data - T:%.1f°C, P:%.1fhPa, HDG:%.0f°\n", 
                          temperature, pressure, heading);
        } else {
            // 硬體失效，標記為不可用
            sensorsHardwareAvailable = false;
            Serial.println("Hardware sensors failed, switching to simulated mode");
        }
    }
    
    // 如果硬體不可用或讀取失敗，使用模擬數據
    if (!dataFromHardware) {
        temperature = 25.5f;
        pressure = 1012.0f;
        altitude = 590.0f; // 520 + 70 = 590m (海拔校正)
        heading = 270.0f;
        compassDir = "W";
        
        Serial.printf("Simulated sensor data - T:%.1f°C, P:%.1fhPa, HDG:%.0f°\n", 
                      temperature, pressure, heading);
    }
    
    // 格式化顯示文本
    char statusText[250];
    snprintf(statusText, sizeof(statusText),
        "T: %.1fC %s\n"         // 溫度 + 狀態指示
        "H: %.0f m\n"           // 海拔  
        "P: %.0f hPa\n"         // 氣壓
        "STEP: %lu\n"           // 計步器
        "HDG: %.0f %s",         // 指南針方向
        temperature, dataFromHardware ? "" : "*",
        altitude,
        pressure,
        stepCount,
        heading,
        compassDir
    );
    
    lv_label_set_text(statusLabel, statusText);
    
    Serial.printf("Status Display - T:%.1f°C, P:%.0fhPa, H:%.0fm, HDG:%.0f° %s, STEP:%lu [%s]\n", 
                  temperature, pressure, altitude, heading, compassDir, stepCount,
                  dataFromHardware ? "HW" : "SIM");
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

float StatusView::calculateAltitude(float pressure, float seaLevelPressure) {
    // 使用國際標準大氣壓力公式計算海拔高度
    // 預設海平面標準氣壓: 1013.25 hPa，但可以指定當地氣壓進行校正
    if (seaLevelPressure <= 0) seaLevelPressure = 1013.25f;
    return 44330.0f * (1.0f - pow(pressure / seaLevelPressure, 0.1903f));
}

float StatusView::calculateAltitude(float pressure) {
    // 重載函數，使用標準海平面氣壓
    return calculateAltitude(pressure, 1013.25f);
}

float StatusView::calculateHeading(float mx, float my) {
    // 計算磁北方向 (度數)
    float heading = atan2(my, mx) * 180.0f / M_PI;
    if (heading < 0) heading += 360.0f;
    return heading;
}

const char* StatusView::getCompassDirection(float heading) {
    // 將角度轉換為方向標示
    if (heading >= 337.5f || heading < 22.5f) return "N";
    else if (heading >= 22.5f && heading < 67.5f) return "NE";
    else if (heading >= 67.5f && heading < 112.5f) return "E";
    else if (heading >= 112.5f && heading < 157.5f) return "SE";
    else if (heading >= 157.5f && heading < 202.5f) return "S";
    else if (heading >= 202.5f && heading < 247.5f) return "SW";
    else if (heading >= 247.5f && heading < 292.5f) return "W";
    else return "NW";
}

void StatusView::updateStepCounter(float ax, float ay, float az) {
    if (!stepCounterAccount) return;
    
    // 獲取當前計步器數據
    DataProc::StepCounter_Info_t stepData = {};
    stepCounterAccount->Pull(stepCounterAccount, &stepData, sizeof(stepData));
    
    // 簡單的計步演算法：檢測加速度向量的變化
    float magnitude = sqrt(ax*ax + ay*ay + az*az);
    uint32_t currentTime = millis();
    
    // 步伐檢測：幅度變化超過閾值且時間間隔合理
    if (abs(magnitude - stepData.lastMagnitude) > 0.3f && 
        (currentTime - stepData.lastStepTime) > 300 && // 最小步伐間隔 300ms
        !stepData.stepDetected) {
        stepData.stepCount++;
        stepData.lastStepTime = currentTime;
        stepData.stepDetected = true;
    }
    
    // 重置檢測狀態
    if (abs(magnitude - stepData.lastMagnitude) < 0.1f) {
        stepData.stepDetected = false;
    }
    
    stepData.lastMagnitude = magnitude;
    
    // 更新 DataProc
    stepCounterAccount->Commit(&stepData, sizeof(stepData));
    stepCounterAccount->Publish();
}

void StatusView::publishSensorData() {
    if (!sensorsAccount) return;
    
    // 讀取 GY-80 傳感器數據
    GY80::BMPData bmpData = gy80.readBMP();
    GY80::MagData magData = gy80.readMag();
    GY80::AccelData accelData = gy80.readAccel();
    
    // 調試信息：檢查傳感器讀取狀態
    Serial.printf("GY80 Status - BMP:%s, MAG:%s, ACCEL:%s\n", 
                  bmpData.ok ? "OK" : "FAIL",
                  magData.ok ? "OK" : "FAIL", 
                  accelData.ok ? "OK" : "FAIL");
    
    if (bmpData.ok) {
        Serial.printf("BMP Data - T:%.1f°C, P:%.1fhPa\n", bmpData.temperature, bmpData.pressure);
    }
    
    if (magData.ok) {
        Serial.printf("MAG Data - X:%.1f, Y:%.1f, Z:%.1f\n", magData.mx, magData.my, magData.mz);
    }
    
    if (accelData.ok) {
        Serial.printf("ACCEL Data - X:%.2f, Y:%.2f, Z:%.2f\n", accelData.ax, accelData.ay, accelData.az);
    }
    
    // 準備傳感器數據結構
    DataProc::Sensors_Info_t sensorsData = {};
    
    // 溫度和氣壓數據
    sensorsData.temperatureValid = bmpData.ok;
    sensorsData.pressureValid = bmpData.ok;
    sensorsData.temperature = bmpData.ok ? bmpData.temperature : 25.5f;
    sensorsData.pressure = bmpData.ok ? bmpData.pressure : 1012.0f;
    sensorsData.altitude = calculateAltitude(sensorsData.pressure);
    
    // 磁力計數據
    sensorsData.magnetometerValid = magData.ok;
    sensorsData.magneticX = magData.ok ? magData.mx : 0.0f;
    sensorsData.magneticY = magData.ok ? magData.my : 0.0f;
    sensorsData.magneticZ = magData.ok ? magData.mz : 0.0f;
    sensorsData.heading = magData.ok ? calculateHeading(magData.mx, magData.my) : 270.0f;
    strncpy(sensorsData.compassDirection, getCompassDirection(sensorsData.heading), 3);
    sensorsData.compassDirection[3] = '\0';
    
    // 加速度計數據
    sensorsData.accelerometerValid = accelData.ok;
    sensorsData.accelX = accelData.ok ? accelData.ax : 0.0f;
    sensorsData.accelY = accelData.ok ? accelData.ay : 0.0f;
    sensorsData.accelZ = accelData.ok ? accelData.az : 0.0f;
    
    // 發布數據到 DataProc
    sensorsAccount->Commit(&sensorsData, sizeof(sensorsData));
    sensorsAccount->Publish();
    
    // 如果有加速度數據，則更新計步器
    if (accelData.ok) {
        updateStepCounter(accelData.ax, accelData.ay, accelData.az);
    }
}

void StatusView::publishStepData() {
    // 計步器數據已在 updateStepCounter 中更新
}

void StatusView::recheckSensorHardware() {
    Serial.println("Rechecking GY-80 sensor hardware...");
    lastSensorCheck = millis();
    
    // 快速 I2C 設備檢查
    bool deviceFound = false;
    
    for (uint8_t addr = 0x1E; addr <= 0x77; addr++) {
        if (addr == 0x1E || addr == 0x53 || addr == 0x77) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                deviceFound = true;
                Serial.printf("Device detected at 0x%02X\n", addr);
                break;
            }
        }
    }
    
    if (deviceFound && !sensorsHardwareAvailable) {
        // 嘗試重新初始化 GY-80
        if (gy80.begin(Wire, 21, 22, 100000)) {
            sensorsHardwareAvailable = true;
            Serial.println("✓ GY-80 hardware reconnected!");
        } else {
            Serial.println("Device found but GY-80 init failed");
        }
    } else if (!deviceFound && sensorsHardwareAvailable) {
        sensorsHardwareAvailable = false;
        Serial.println("✗ GY-80 hardware disconnected");
    }
}