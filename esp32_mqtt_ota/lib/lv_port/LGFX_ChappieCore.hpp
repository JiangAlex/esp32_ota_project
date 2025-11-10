#ifndef LGFX_CHAPPIE_CORE_HPP
#define LGFX_CHAPPIE_CORE_HPP

#include <LovyanGFX.hpp>

// LovyanGFX configuration for SSD1306 128x64 OLED I2C display
// Pin connections: SDA=21, SCL=22, I2C Address=0x3C
// Note: This display has color zones - Yellow(top 16px) + Blue(bottom 48px)
class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_SSD1306 _panel_instance;
  lgfx::Bus_I2C _bus_instance;

public:
  LGFX(void)
  {
    { // I2C bus configuration for OLED
      auto cfg = _bus_instance.config();
      cfg.i2c_port = 0;             // I2C port number (0 or 1)
      cfg.freq_write = 400000;      // I2C frequency (400kHz)
      cfg.freq_read = 400000;       // I2C frequency for reading
      cfg.pin_sda = 21;             // I2C SDA pin
      cfg.pin_scl = 22;             // I2C SCL pin
      cfg.i2c_addr = 0x3C;          // I2C address for SSD1306 (0x3C or 0x3D)
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // SSD1306 OLED panel configuration
      auto cfg = _panel_instance.config();
      cfg.pin_cs = -1;              // Not used for I2C
      cfg.pin_rst = -1;             // Reset pin (optional, set to actual pin if connected)
      cfg.pin_busy = -1;            // Not used

      // OLED display dimensions
      cfg.memory_width = 128;
      cfg.memory_height = 64;
      cfg.panel_width = 128;
      cfg.panel_height = 64;

      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 2;      // 先試試原始方向

      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;         // OLED is write-only
      cfg.invert = false;          // 關閉反轉顯示（保持旋轉180度）
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};

#endif // LGFX_CHAPPIE_CORE_HPP