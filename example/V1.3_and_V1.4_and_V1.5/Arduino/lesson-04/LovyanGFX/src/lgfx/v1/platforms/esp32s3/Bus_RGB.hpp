#pragma once

#if __has_include (<esp_lcd_panel_rgb.h>)
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>

#include "../../Bus.hpp"
#include "../../panel/Panel_FrameBufferBase.hpp"

namespace lgfx
{
 inline namespace v1
 {
  class Bus_RGB : public IBus
  {
  public:
    struct config_t
    {
      Panel_FrameBufferBase* panel = nullptr;
      int8_t port = 0;
      uint32_t freq_write = 16000000;
      int8_t pin_pclk = -1;
      int8_t pin_vsync = -1;
      int8_t pin_hsync = -1;
      int8_t pin_henable = -1;
      union
      {
        int8_t pin_data[16];
        struct
        {
          int8_t pin_d0;
          int8_t pin_d1;
          int8_t pin_d2;
          int8_t pin_d3;
          int8_t pin_d4;
          int8_t pin_d5;
          int8_t pin_d6;
          int8_t pin_d7;
          int8_t pin_d8;
          int8_t pin_d9;
          int8_t pin_d10;
          int8_t pin_d11;
          int8_t pin_d12;
          int8_t pin_d13;
          int8_t pin_d14;
          int8_t pin_d15;
        };
      };
      int8_t hsync_pulse_width = 0;
      int8_t hsync_back_porch = 0;
      int8_t hsync_front_porch = 0;
      int8_t vsync_pulse_width = 0;
      int8_t vsync_back_porch = 0;
      int8_t vsync_front_porch = 0;
      bool hsync_polarity = 0;
      bool vsync_polarity = 0;
      bool pclk_active_neg = 1;
      bool de_idle_high = 0;
      bool pclk_idle_high = 0;
    };

    const config_t& config(void) const { return _cfg; }
    void config(const config_t& config);

    bus_type_t busType(void) const override { return bus_type_t::bus_unknown; }
    bool init(void) override;
    void release(void) override;

    void beginTransaction(void) override {}
    void endTransaction(void) override {}
    void wait(void) override {}
    bool busy(void) const override { return false; }
    void flush(void) override {}
    bool writeCommand(uint32_t, uint_fast8_t) override { return true; }
    void writeData(uint32_t, uint_fast8_t) override {}
    void writeDataRepeat(uint32_t, uint_fast8_t, uint32_t) override {}
    void writePixels(pixelcopy_t*, uint32_t) override {}
    void writeBytes(const uint8_t*, uint32_t, bool, bool) override {}

    void initDMA(void) override {}
    void addDMAQueue(const uint8_t*, uint32_t) override {}
    void execDMAQueue(void) override {}
    uint8_t* getDMABuffer(uint32_t) override;

    uint8_t* getBackBuffer(void);
    void present(void);

    void beginRead(void) override {}
    void endRead(void) override {}
    uint32_t readData(uint_fast8_t) override { return 0; }
    bool readBytes(uint8_t*, uint32_t, bool) override { return false; }
    void readPixels(void*, pixelcopy_t*, uint32_t) override {}

  private:
    static bool onVsync(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t* data, void* user_ctx);
    static bool onBounceEmpty(esp_lcd_panel_handle_t panel, void* bounce_buf, int pos_px, int len_bytes, void* user_ctx);

    config_t _cfg;
    esp_lcd_panel_handle_t _panel_handle = nullptr;
    uint8_t* _frame_buffer[2] = { nullptr, nullptr };
    volatile uint8_t _active_buffer = 0;
    volatile int8_t _pending_buffer = -1;
  };
 }
}
#endif
