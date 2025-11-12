#ifndef __HAL_CONFIG_H
#define __HAL_CONFIG_H

/* Display Configuration */
#define DISPLAY_WIDTH           128
#define DISPLAY_HEIGHT          64
#define DISPLAY_I2C_SDA         21
#define DISPLAY_I2C_SCL         22
#define DISPLAY_I2C_ADDRESS     0x3C
#define DISPLAY_BUFFER_LINES    8

/* Button Configuration */
#define BTN_MENU_OK_PIN         32  // GPIO 32 - Menu/OK 按鈕
#define BTN_UP_BACK_PIN         33  // GPIO 33 - UP/Back 按鈕
#define BTN_DOWN_FN_PIN         34  // GPIO 34 - Down/Fn 按鈕 (輸入專用，需外部上拉)

#define BTN_DEBOUNCE_DELAY      50   // 50ms去抖動
#define BTN_HOLD_TIME           2000 // 2000ms長按 (2秒)
#define BTN_DOUBLE_CLICK_TIME   300  // 300ms雙擊間隔

/* SA818 Configuration */
#define SA818_RX_PIN            15   // arduino serial RX pin to the DRA818 TX pin 17
#define SA818_TX_PIN            16   // arduino serial TX pin to the DRA818 RX pin 16
#define SA818_PD_PIN            -1   // to the DRA818 PD pin 6 (disabled)
#define SA818_BAUD              9600 // Default baud rate
#define SA818_BAUD_ALT          115200 // Alternative baud rate

/* Power Configuration */
#define POWER_BATTERY_PIN       36   // ADC pin for battery monitoring
#define POWER_CHARGE_PIN        39   // Charging status pin
#define POWER_LOW_VOLTAGE       3.2  // Low battery voltage threshold

/* I2C Configuration */
#define I2C_SDA_PIN             21
#define I2C_SCL_PIN             22
#define I2C_FREQUENCY           100000

/* Update Intervals (ms) */
#define HAL_UPDATE_INTERVAL     10
#define BUTTON_UPDATE_INTERVAL  20
#define POWER_UPDATE_INTERVAL   500
#define STATUS_UPDATE_INTERVAL  2000
#define TIME_UPDATE_INTERVAL    5000

/* LVGL Configuration */
#define LVGL_BUFFER_SIZE        (DISPLAY_WIDTH * DISPLAY_BUFFER_LINES)

#endif // __HAL_CONFIG_H