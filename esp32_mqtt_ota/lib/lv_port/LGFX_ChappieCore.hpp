#ifndef LGFX_CHAPPIE_CORE_HPP
#define LGFX_CHAPPIE_CORE_HPP

#include <LovyanGFX.hpp>

// LovyanGFX configuration for ILI9341 - ChappieCore board
class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM     _light_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;     // Use SPI2_HOST
      cfg.spi_mode = 0;
      cfg.freq_write = 80000000;    // SPI clock for writing (up to 80MHz)
      cfg.freq_read = 16000000;     // SPI clock for reading
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 14;   // SPI SCLK pin
      cfg.pin_mosi = 13;   // SPI MOSI pin
      cfg.pin_miso = 12;   // SPI MISO pin (not used for display)
      cfg.pin_dc = 2;      // Data/Command pin
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 15;     // Chip Select pin
      cfg.pin_rst = -1;     // Reset pin
      cfg.pin_busy = -1;   // Busy pin (not used)
      cfg.memory_width = 240;
      cfg.memory_height = 320;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }

    { // バックライト制御の設定を行います。（必要なければ削除）
      auto cfg = _light_instance.config();    // バックライト設定用の構造体を取得します。

      cfg.pin_bl = 21;              // バックライトが接続されているピン番号
      cfg.invert = false;           // バックライトの輝度を反転させる場合 true
      cfg.freq   = 44100;           // バックライトのPWM周波数
      cfg.pwm_channel = 7;          // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // バックライトをパネルにセットします。
    }

    setPanel(&_panel_instance);
  }
};

#endif // LGFX_CHAPPIE_CORE_HPP