#if defined (ESP_PLATFORM)
#include <sdkconfig.h>
#if defined (CONFIG_IDF_TARGET_ESP32S3)
#if __has_include (<esp_lcd_panel_rgb.h>)
#include "Bus_RGB.hpp"
#include "../common.hpp"
#include <string.h>

namespace lgfx
{
 inline namespace v1
 {
  void Bus_RGB::config(const config_t& cfg)
  {
    _cfg = cfg;
  }

  bool IRAM_ATTR Bus_RGB::onVsync(esp_lcd_panel_handle_t, const esp_lcd_rgb_panel_event_data_t*, void* user_ctx)
  {
    return false;
  }

  bool IRAM_ATTR Bus_RGB::onBounceEmpty(esp_lcd_panel_handle_t, void* bounce_buf, int pos_px, int len_bytes, void* user_ctx)
  {
    auto bus = static_cast<Bus_RGB*>(user_ctx);
    if (pos_px == 0) {
      int8_t pending = bus->_pending_buffer;
      if (pending >= 0 && pending != bus->_active_buffer) {
        bus->_active_buffer = (uint8_t)pending;
        bus->_pending_buffer = -1;
      }
    }
    const uint8_t* source = bus->_frame_buffer[bus->_active_buffer] + ((size_t)pos_px * 2);
    memcpy(bounce_buf, source, (size_t)len_bytes);
    return false;
  }

  bool Bus_RGB::init(void)
  {
    const uint32_t width = _cfg.panel->width();
    const uint32_t height = _cfg.panel->height();
    const size_t frame_bytes = width * height * 2;

    _frame_buffer[0] = (uint8_t*)heap_alloc_psram(frame_bytes);
    _frame_buffer[1] = (uint8_t*)heap_alloc_psram(frame_bytes);
    if (!_frame_buffer[0] || !_frame_buffer[1]) {
      release();
      return false;
    }
    memset(_frame_buffer[0], 0, frame_bytes);
    memset(_frame_buffer[1], 0, frame_bytes);

    esp_lcd_rgb_panel_config_t panel_config = {};
    panel_config.clk_src = LCD_CLK_SRC_DEFAULT;
    panel_config.timings.pclk_hz = _cfg.freq_write;
    panel_config.timings.h_res = width;
    panel_config.timings.v_res = height;
    panel_config.timings.hsync_pulse_width = _cfg.hsync_pulse_width;
    panel_config.timings.hsync_back_porch = _cfg.hsync_back_porch;
    panel_config.timings.hsync_front_porch = _cfg.hsync_front_porch;
    panel_config.timings.vsync_pulse_width = _cfg.vsync_pulse_width;
    panel_config.timings.vsync_back_porch = _cfg.vsync_back_porch;
    panel_config.timings.vsync_front_porch = _cfg.vsync_front_porch;
    panel_config.timings.flags.hsync_idle_low = !_cfg.hsync_polarity;
    panel_config.timings.flags.vsync_idle_low = !_cfg.vsync_polarity;
    panel_config.timings.flags.de_idle_high = _cfg.de_idle_high;
    panel_config.timings.flags.pclk_active_neg = _cfg.pclk_active_neg;
    panel_config.timings.flags.pclk_idle_high = _cfg.pclk_idle_high;
    panel_config.data_width = 16;
    panel_config.bits_per_pixel = 16;
    panel_config.bounce_buffer_size_px = width * 16;
    panel_config.dma_burst_size = 64;
    panel_config.hsync_gpio_num = _cfg.pin_hsync;
    panel_config.vsync_gpio_num = _cfg.pin_vsync;
    panel_config.de_gpio_num = _cfg.pin_henable;
    panel_config.pclk_gpio_num = _cfg.pin_pclk;
    panel_config.disp_gpio_num = -1;
    for (uint8_t i = 0; i < 16; ++i) {
      panel_config.data_gpio_nums[i] = _cfg.pin_data[i ^ 8];
    }
    panel_config.flags.no_fb = 1;

    if (esp_lcd_new_rgb_panel(&panel_config, &_panel_handle) != ESP_OK) {
      release();
      return false;
    }

    esp_lcd_rgb_panel_event_callbacks_t callbacks = {};
    callbacks.on_vsync = onVsync;
    callbacks.on_bounce_empty = onBounceEmpty;
    if (esp_lcd_rgb_panel_register_event_callbacks(_panel_handle, &callbacks, this) != ESP_OK) {
      release();
      return false;
    }
    if (esp_lcd_panel_reset(_panel_handle) != ESP_OK || esp_lcd_panel_init(_panel_handle) != ESP_OK) {
      release();
      return false;
    }
    return true;
  }

  uint8_t* Bus_RGB::getDMABuffer(uint32_t)
  {
    return _frame_buffer[0];
  }

  uint8_t* Bus_RGB::getBackBuffer(void)
  {
    return _frame_buffer[_active_buffer ^ 1];
  }

  void Bus_RGB::present(void)
  {
    while (_pending_buffer >= 0) {
      delay(1);
    }
    _pending_buffer = (int8_t)(_active_buffer ^ 1);
  }

  void Bus_RGB::release(void)
  {
    if (_panel_handle) {
      esp_lcd_panel_del(_panel_handle);
      _panel_handle = nullptr;
    }
    for (uint8_t i = 0; i < 2; ++i) {
      if (_frame_buffer[i]) {
        heap_free(_frame_buffer[i]);
        _frame_buffer[i] = nullptr;
      }
    }
  }
 }
}
#endif
#endif
#endif
