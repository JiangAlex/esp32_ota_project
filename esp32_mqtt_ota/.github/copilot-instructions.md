# ESP32 OLED Display Application - Copilot Instructions

## Architecture Overview

This is an ESP32-based OLED display application using **MVP (Model-View-Presenter) architecture** with LVGL graphics library. The system manages multiple pages through a centralized `PageManager` singleton and handles 3-button navigation.

### Core Components

- **Hardware**: SSD1306 128x64 OLED (I2C: SDA=21, SCL=22, Addr=0x3C), 3 GPIO buttons
- **Display**: LovyanGFX + LVGL stack with color zones (top 16px yellow, bottom 48px blue)
- **Architecture**: MVP pattern with singleton managers for pages and buttons
- **Radio**: DRA818 VHF/UHF transceiver integration for walkie-talkie functionality

## Key Patterns & Conventions

### Page Management System
All pages inherit from MVP base classes and are managed by `PageManager::getInstance()`:
```cpp
// Page switching pattern
PageManager* pm = PageManager::getInstance();
pm->switchToPage(PAGE_TREKKING);
```

Pages follow enum: `PAGE_MAINMENU`, `PAGE_TREKKING`, `PAGE_WALKIETALKIE`, `PAGE_STATUS`, `PAGE_SYSTEM`

### Button Event Handling
3-button system with sophisticated event detection:
- **GPIO 32**: Menu/OK (short: context action, long: return to main menu)
- **GPIO 33**: UP/Back (navigation or scroll up in scroll mode)  
- **GPIO 34**: Down/Fn (navigation or scroll down, double-click for functions)

Button events: `BTN_EVENT_PRESS`, `BTN_EVENT_HOLD`, `BTN_EVENT_DOUBLE`

### Display Architecture
- **LGFX_ChappieCore.hpp**: Hardware-specific OLED configuration
- **lv_port/**: LVGL integration layer
- Two-zone display: status bar (16px) + content area (48px)

### MVP Implementation Example
```cpp
class ExamplePresenter : public BasePresenter<ExampleView> {
    ExampleModel* model;
public:
    void onCreate() override; // Initialize
    void onShow() override;   // Page activated
    void onHide() override;   // Page deactivated
};
```

## Development Workflows

### Build & Flash
```bash
# PlatformIO commands (use these, not Arduino IDE)
pio run                    # Build
pio run --target upload    # Flash to ESP32
pio run --target monitor   # Serial monitor
```

### Hardware Dependencies
- **Required**: External 10kΩ pull-up resistor on GPIO 34 (input-only pin)
- **Display**: Verify I2C connections before debugging display issues
- **Power**: 3.3V logic levels for DRA818 (use level shifter if needed)

## Radio Integration (DRA/SA818)

### Frequency Management
The system supports dual power modes for SA818:
- **L-CH (Low Power)**: 409.75-409.9875 MHz (20 channels)
- **H-CH (High Power)**: 430.1375-439.4375 MHz (20 channels)

### Radio Configuration Pattern
```cpp
// DRA818 initialization pattern from lib/arduino-dra818-master/
DRA818::configure(serial, DRA818_UHF, rx_freq, tx_freq, squelch, volume, 
                  ctcss_rx, ctcss_tx, bandwidth, pre, high, low);
```

## Code Navigation

### Key Directories
- `src/App/Pages/*/`: MVP triads for each page
- `src/App/Utils/PageManager/`: Central page orchestration  
- `src/App/Utils/ButtonManager.*`: Input event handling
- `lib/lv_port/`: Display hardware abstraction
- `src/App/Common/HAL/`: Hardware abstraction layer

### Critical Files
- `main.cpp`: Application entry point and main loop
- `PageManager.h`: Page enum definitions and component access
- `LGFX_ChappieCore.hpp`: OLED-specific display configuration

## Common Gotchas

- **GPIO 34**: Input-only pin requiring external pull-up resistor
- **LVGL Timing**: Always call `lv_timer_handler()` in main loop
- **Display Zones**: SSD1306 has fixed color zones (yellow/blue) - design accordingly  
- **Button Debouncing**: Built into ButtonManager with 50ms debounce + hold/double-click detection
- **Memory Management**: Use singleton pattern for managers, careful with LVGL object lifecycle

## Testing & Debugging

- Serial output at 115200 baud provides detailed button events and page transitions
- OLED initialization status logged on startup
- Page switching logged with current/previous page info
- Button event types logged for debugging input handling