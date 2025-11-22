/*
  ESP32 OLED Display Application with 3-Button Control
  
  This application provides a 128x64 OLED display interface using LVGL with LovyanGFX.
  Features include 3-button navigation, page management with MVP architecture, and visual display.
  The application supports multiple pages (Menu, Settings, Status) with button control.
  
  Hardware Configuration:
  - Display: SSD1306 128x64 OLED (I2C: SDA=21, SCL=22, Addr=0x3C)
  - Button 1: GPIO 32 (Menu/OK) - Short: Menu, Long: Settings
  - Button 2: GPIO 33 (UP/Back) - Short: Previous page, Long: Status  
  - Button 3: GPIO 34 (Down/Fn) - Short: Next page, Long: Reserved, Double: Function
  
  注意: GPIO 34需要外部10kΩ上拉電阻連接到3.3V
  
  Display Layout:
  - Top 16px (Yellow zone): Page titles and status
  - Bottom 48px (Blue zone): Page content
*/

#include <Arduino.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include "LGFX_ChappieCore.hpp"
#include "App/Utils/PageManager/PageManager.h"
#include "App/Pages/Status/StatusView.h"
#include "App/Pages/Status/StatusPresenter.h"
#include "App/Pages/Settings/SettingsView.h"
#include "App/Pages/Settings/SettingsPresenter.h"
#include "App/Pages/Menu/MenuPresenter.h"


void setup() {
  Serial.begin(115200);
  delay(100);

}

void loop() {

}