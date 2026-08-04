# PlatformIO + LVGL 9.1 Upgrade Guide

## Fixed toolchain

| Item | Version |
| --- | --- |
| PlatformIO Core | 6.1.19 or newer |
| pioarduino platform-espressif32 | 55.03.39 |
| Arduino-ESP32 | 3.3.9 |
| ESP-IDF libraries | 5.5.4 |
| Xtensa toolchain | 14.2.0 (2026-01-21) |
| LVGL | 9.1.0 |
| LovyanGFX | 1.2.26 |

The platform URL and every project library are pinned in `platformio.ini`.
PlatformIO resolves and downloads the official libraries; no third-party source
has been patched.

## Build

Install PlatformIO Core 6.1.19 or newer, then run from this directory:

```powershell
pio run
```

Upload and monitor commands:

```powershell
pio run --target upload
pio device monitor --baud 115200
```

On Windows, enable Win32 long-path support. The Arduino-ESP32 package contains
Matter headers whose extracted paths can exceed the legacy 260-character limit.
As a temporary alternative, set `PLATFORMIO_CORE_DIR` to a short path such as
`C:\temp\pio` before the first build.

## Runtime checks

1. Confirm that I2C devices `0x30` and `0x5D` are detected.
2. Confirm that the 800 x 480 RGB panel displays the SquareLine screen.
3. Confirm touch coordinates and both On/Off buttons.
4. Confirm GPIO 19 changes state when the buttons are released.
5. Observe several minutes of full-screen refresh for tearing or corruption.
6. Confirm that both 76,800-byte LVGL partial buffers are allocated from PSRAM.

The firmware aborts with a serial message if either display buffer cannot be
allocated. The display uses two 48-line partial buffers, so a button update does
not redraw and transfer the complete 800 x 480 frame. DMA completion is awaited
before LVGL is told that a flush finished, so LVGL cannot reuse an active buffer.

## Generated firmware

A successful build creates:

- `.pio/build/advance-hmi/firmware.bin`
- `.pio/build/advance-hmi/firmware.factory.bin`
- `.pio/build/advance-hmi/firmware.elf`

`firmware.factory.bin` is the combined image for address `0x0`.
