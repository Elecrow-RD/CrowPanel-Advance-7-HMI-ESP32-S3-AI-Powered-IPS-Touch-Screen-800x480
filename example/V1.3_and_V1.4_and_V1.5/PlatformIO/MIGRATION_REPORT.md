# LVGL 9.1 Migration Report

## Scope and result

The project was migrated from a mixed LVGL 8.3.11/9.1 state to LVGL 9.1.0.
The build uses pioarduino 55.03.39 with Arduino-ESP32 3.3.9 and ESP-IDF 5.5.4.
All original UI, display, touch, I2C, backlight and GPIO behavior remains present.

The final release build completed without compiler or linker errors. It used
815,031 bytes of flash and 91,012 bytes of statically measured RAM in the initial
build. Two 80-line RGB565 partial buffers are allocated dynamically from PSRAM
at startup to avoid full-screen transfers for small UI updates. The GPIO controls
use fixed visual styles, avoiding unnecessary framebuffer writes on touch-state
changes in the single-buffer RGB driver.

## File changes

| File | Change |
| --- | --- |
| `platformio.ini` | Pinned the platform, LVGL 9.1.0 and LovyanGFX 1.2.26; pinned TAMC_GT911; exposed `lv_conf.h` to library compilation. |
| `src/main.cpp` | Replaced LVGL 8 display/input drivers with LVGL 9.1 objects and callbacks; added tick callback, partial RGB565 buffers, allocation checks and DMA completion synchronization. |
| `include/lv_conf.h` | Added project-owned LVGL 9.1 configuration with 16-bit color and Montserrat 40. |
| `include/ui.h` | Removed obsolete SquareLine/LVGL 8.3 duplicate. |
| `include/ui_helpers.h` | Removed obsolete SquareLine/LVGL 8.3 duplicate. |
| `include/ui_events.h` | Removed obsolete SquareLine/LVGL 8.3 duplicate. |
| `.gitignore` | Excluded build, IDE and local tool caches. |
| `UPGRADE_GUIDE.md` | Added build, upload and hardware validation instructions. |
| `MIGRATION_REPORT.md` | Added this migration record. |

No file under `.pio/libdeps` or any other third-party library was manually
modified.

## LVGL API mapping

| LVGL 8 API/type | LVGL 9.1 replacement |
| --- | --- |
| `lv_disp_draw_buf_t` | Buffers owned by `lv_display_t` |
| `lv_disp_draw_buf_init` | `lv_display_set_buffers` |
| `lv_disp_drv_t` and `lv_disp_drv_register` | `lv_display_create` plus display setters |
| `lv_disp_flush_ready` | `lv_display_flush_ready` |
| `lv_color_t *color_p` flush data | `uint8_t *px_map` |
| `lv_indev_drv_t` and `lv_indev_drv_register` | `lv_indev_create` plus input setters |
| LVGL 8 custom tick macros | `lv_tick_set_cb` |

SquareLine Studio 1.5.3 already generated the screen, image descriptor, widget,
event, style, theme and helper APIs for LVGL 9.1, so those source files were kept.

Official references:

- LVGL 9.1 display integration: https://docs.lvgl.io/9.1/porting/display.html
- LVGL 9.1 input integration: https://docs.lvgl.io/9.1/porting/indev.html
- LVGL 9.1 tick interface: https://docs.lvgl.io/9.1/porting/tick.html
- LVGL 9.0 migration guide: https://docs.lvgl.io/9.1/CHANGELOG.html
- pioarduino 55.03.39 release: https://github.com/pioarduino/platform-espressif32/releases/tag/55.03.39
- LovyanGFX 1.2.26 release: https://github.com/lovyan03/LovyanGFX/releases/tag/1.2.26

## Libraries

| Library | Final version | Disposition |
| --- | --- | --- |
| LVGL | 9.1.0 | Mandatory upgrade; exact version pinned. |
| LovyanGFX | 1.2.26 | Upgraded to the latest official stable release. |
| Adafruit BusIO | 1.17.0 | Retained unchanged. |
| Adafruit SSD1306 | 2.5.13 | Retained unchanged. |
| Adafruit GFX | 1.12.6 | Resolved as the SSD1306 dependency. |
| TAMC_GT911 | 1.0.2 | Retained and exactly pinned; active touch remains LovyanGFX GT911. |
| TCA9554 | 0.1.1 | Retained unchanged. |

## Remaining hardware validation

Compilation verifies API, component and linker compatibility but cannot verify
electrical behavior. The RGB timing, PSRAM stability at the configured speed,
touch orientation, I2C reset sequence and physical GPIO output require testing
on the target ESP32-S3 board. No feature was removed to avoid these checks.
