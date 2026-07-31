# 7.0-Inch Hardware Identity Product Hardware Driver Guide

| Item | Details |
|---|---|
| Document Version | V1.0 |
| Corresponding Hardware Documentation | `ESP32 Display 7.0 inch V1.5.sch/.brd`, `ESP32-Display-7.0-inch-V1.5.pdf` |
| Preparation Date | 2026-07-30 |
| Author | OpenAI Codex (cross-compiled from project materials) |
| Scope | The 7.0-inch CrowPanel Advance HMI hardware in this repository and the accompanying Arduino validation projects |

> **Priority of conclusions:** Board-level driver code that has been successfully tested or delivered with the project > EAGLE `.sch` net connections > PDF visual annotations > general-purpose chip datasheets. This document uses the pins and parameters actually used by the code as the porting baseline. Functions confirmed only by the schematic and lacking validation programs in the repository are explicitly marked “To Be Verified” and must not be treated as production software acceptance results.

## 1. Document Sources and Evidence Levels

### 1.1 Input Materials

1. `version1.5/ESP32 Display 7.0 inch V1.5.sch`: Primary source for components, net names, and electrical connections.
2. `version1.5/ESP32-Display-7.0-inch-V1.5.pdf`: Source for functional sections, DIP-switch instructions, and visual verification.
3. `Arduino/lesson-01` through `lesson-09`: Examples for serial communication, audio, LCD/LVGL, TF card, expansion ports, nRF24L01, SX1262/LoRaWAN, and UART/Zigbee.
4. `Arduino/SD_CrowPanel_ESP32_Advance_HMI_4_3_5_0_7_0`: An additional TF card display example, used only to identify differences in historical parameters.
5. Library versions included with the repository: Arduino-ESP32 3.3.8 (specified in the README), LovyanGFX 1.2.25, LVGL 9.1.0, ESP32-audioI2S 3.4.7, RF24 1.6.1, and RadioLib 7.7.1.

### 1.2 Evidence Levels

| Level | Meaning | Usage Rule |
|---|---|---|
| A: Code-verified | Complete initialization and application read/write operations exist in the project examples | May be used directly as the porting baseline, but regression testing in the target firmware is still required |
| B: Schematic-confirmed | Complete connections exist in the schematic, but the repository has no corresponding read/write example | Pin connections are reliable; protocol parameters require first-board validation |
| C: External-module example | The example targets a module connected to an expansion port and does not represent an onboard component | Applicable only when the corresponding peripheral is installed and the DIP switches are configured correctly |
| D: Inferred/To Be Confirmed | Conflicts exist in model, address, or production-batch information | Must be verified against the physical unit and chip datasheet before use |

## 2. Product Architecture and Peripheral Overview

The main controller is an ESP32-S3-WROOM-1 module. The MCU section title in the schematic PDF is labeled `ESP32-S3-N16R8`, indicating that the target configuration is likely 16 MB Flash + 8 MB PSRAM; however, the laser marking on the physical module and the Arduino build menu must take precedence. The board also includes an STC8H1K28 auxiliary control MCU, which handles backlight control, touch reset, buzzer control, amplifier/board-level power control, and other functions over a shared I²C bus.

| Category | Component/Interface | Primary Connections | Driver Method | Level |
|---|---|---|---|---|
| Main Controller | ESP32-S3-WROOM-1 (shown as N16R8) | All main-controller GPIOs; USB-UART | Arduino-ESP32 / ESP-IDF HAL | A/D |
| Board Control MCU | STC8H1K28-36I-LQFP32 | I²C GPIO15/16, address `0x30`; GPIO1 recovery path | I²C single-byte commands | A |
| Display | 7.0-inch 800×480 RGB IPS | GPIO3/7/9~14/17/18/21/38/45~48, 39~42 | 16-bit RGB parallel interface + DMA | A |
| Touch | GT911 capacitive touch | SDA15, SCL16; INT to GPIO1; RST controlled by board control MCU | I²C at 400 kHz, address `0x5D` | A |
| Backlight | MT9201 boost constant-current driver + board control MCU | LCD LEDA/LEDK; MCU controls EN/power | I²C commands 0~245 | A |
| TF Card | MicroSD card slot | GPIO4 MISO, 5 SCK, 6 MOSI; CS fixed in hardware | SPI/FS/SD, 40 MHz | A |
| Audio Output | NS4168 digital-input amplifier + speaker connector | GPIO4 SDATA, 5 BCLK, 6 LRCLK | I²S; board control command `248` enables output | A |
| Digital Microphone | LMD3526B261 PDM MIC | GPIO19 CLK, GPIO20 DATA, through CH486F | PDM/I²S RX | B |
| RTC | PCF8563-compatible component + 32.768 kHz + CR1220 | SDA15, SCL16 | I²C, typically address `0x51` | B/D |
| Buzzer | 5025 passive buzzer + NPN | Controlled by STC8H1K28 P2.7 | Auxiliary MCU PWM/GPIO | B |
| Wireless Expansion | J9/J12 expansion ports | SPI GPIO4/5/6; control GPIO19/20, plus GPIO2/8 | SPI + GPIO/IRQ | C |
| nRF24L01 | External 2.4 GHz module | MISO4/MOSI6/SCK5, CSN19, CE20 | RF24; 250 kbps, CH50 | C |
| SX1262 | External LoRa module | NSS8, DIO1 20, RST19, BUSY2, SPI4/5/6 | RadioLib / LoRaWAN | C |
| UART Expansion | J2/J14/multiplexed port | UART0; or GPIO19 RX, GPIO20 TX | UART 8N1, example uses 115200 | A/C |
| I²C Expansion | J13/J11 | SDA15, SCL16, 3.3 V/GND | I²C, shared onboard bus | A/C |
| GPIO Expansion | J11 | GPIO2, GPIO8, I²C, power | 3.3 V GPIO | B/C |
| USB/Programming | USB-C + CH340K + automatic programming circuit | ESP UART0, EN, GPIO0 | USB-UART / automatic reset and programming | A |
| Battery/Charging | TP4059 + battery connector | USB/5 V, VBAT, CHRG/DONE to board control MCU | Hardware charging, MCU status acquisition | B |
| Main Power | RY3420/HM3416H DC-DC, etc. | VIN → 3.3 V; separate boost supply for display backlight | Hardware power management | B |
| Wi-Fi/BLE | ESP32-S3 integrated radio | Module antenna | Arduino WiFi/BLE stack | A (Wi-Fi) |
| Indicators/Buttons | BOOT, RESET, power/charging LEDs | GPIO0, EN; board control MCU | Active-low buttons/hardware indicators | A/B |

## 3. ESP32-S3 GPIO Baseline Table

| GPIO | Actual Function in Code | Schematic Net/Connection | Multiplexing and Considerations |
|---:|---|---|---|
| 0 | CS parameter passed to the SD library by the TF example; BOOT | `IO0_BOOT`, active-low BOOT button | **Boot strapping pin**; MicroSD CS is not physically connected to GPIO0 and must not be assumed to be controllable as a chip-select signal |
| 1 | Touch/board-control fault recovery: low pulse for 120 ms, then configured as input | `IO1_TP_INT`, touch FPC INT | No touch interrupt is registered at runtime; must not be continuously driven in push-pull mode |
| 2 | SX1262 BUSY; GPIO expansion | J11 GPIO; schematic `IO2` | Used as an input with external LoRa; consider boot state and peripheral signal levels |
| 3 | LCD R3 (logical R3 in code, physical panel R6) | `IO3_R6` | Dedicated to LCD; do not reuse |
| 4 | SD MISO / I²S SDATA / external SPI MISO | Through CH486F analog switch | Mutually exclusive; selected by DIP switch |
| 5 | SD SCK / I²S BCLK / external SPI SCK | Through CH486F analog switch | Mutually exclusive; clock lines must not be connected to multiple channels simultaneously |
| 6 | SD MOSI / I²S LRCLK / external SPI MOSI | Through CH486F analog switch | Mutually exclusive |
| 7 | LCD R0 (panel R3) | `IO7_R3` | Dedicated to LCD |
| 8 | SX1262 NSS; GPIO expansion | J11 GPIO; schematic `IO8` | Used as chip select in the LoRa example |
| 9~14 | LCD G0~G5 (panel G2~G7) | RGB data | Dedicated to LCD |
| 15 | I²C SDA | RTC, GT911, STC, I²C port | External open-drain, pulled up to 3.3 V; shared bus |
| 16 | I²C SCL | RTC, GT911, STC, I²C port | External open-drain, pulled up to 3.3 V; shared bus |
| 17,18 | LCD R1,R2 | RGB data | Dedicated to LCD |
| 19 | Expansion IRQ/CSN/RST/UART RX or PDM CLK | Through CH486F to wireless port/microphone | Multiplexed as a pair with GPIO20; enable only after configuring the DIP switches |
| 20 | Expansion CE/DIO1/UART TX or PDM DATA | Through CH486F to wireless port/microphone | Multiplexed as a pair with GPIO19 |
| 21 | LCD B0 | RGB data | Dedicated to LCD |
| 38,45,47,48 | LCD B4,B3,B1,B2 | RGB data | Dedicated to LCD; GPIO45/46 are high-numbered/strapping-related pins, so retain the validated configuration |
| 39 | LCD PCLK/DCLK | Through 0 Ω/series position to FPC | 18 MHz baseline, idle high |
| 40 | LCD HSYNC | FPC HSYNC | Active low |
| 41 | LCD VSYNC | FPC VSYNC | Active low |
| 42 | LCD DE | FPC DE | Data enable |
| 46 | LCD R4 | `IO46_R7` | Driven by the RGB peripheral matrix on this board and verified by code; this pin is also a high-numbered/startup-configuration-sensitive GPIO, so preserve the existing mapping and recheck the chip datasheet when porting the low-level implementation |

## 4. Driver Details by Peripheral

### 4.1 ESP32-S3-WROOM-1 Main Controller

- **Electrical connections:** 3.3 V supply; active-high `EN`; UART0 connects to USB-C through a CH340K and a BSS138 level-shifting/isolation network; DTR/RTS automatically control `EN` and GPIO0 through a UMH3NTN.
- **Software layer:** Arduino-ESP32 3.3.8, with ESP-IDF GPIO, I²C, SPI, I²S, LCD DMA, and Wi-Fi drivers underneath.
- **Storage:** The schematic contains no separate Flash or PSRAM components; both are integrated into the WROOM module. The PDF title specifies N16R8, but the component name is only WROOM-1, making the exact configuration subject to production-batch verification.
- **Initialization:** It is recommended to call `Serial.begin(115200)` first, then initialize the shared I²C bus/board control MCU, followed by the display or target peripheral. Large frame buffers depend on PSRAM.

### 4.2 RGB LCD (800×480)

**Code baseline:** `Arduino/lesson-04/.../LovyanGFX_Driver.h`. The panel uses RGB565 with 16 data lines. The color comments for `d0..d15` in the code refer to logical controller bits. On the schematic, the FPC connects only the significant color bits: R3~R7, G2~G7, and B3~B7, while the low-order bits are grounded.

| Signal | GPIO | Signal | GPIO | Signal | GPIO |
|---|---:|---|---:|---|---:|
| B0/d0 | 21 | B1/d1 | 47 | B2/d2 | 48 |
| B3/d3 | 45 | B4/d4 | 38 | G0/d5 | 9 |
| G1/d6 | 10 | G2/d7 | 11 | G3/d8 | 12 |
| G4/d9 | 13 | G5/d10 | 14 | R0/d11 | 7 |
| R1/d12 | 17 | R2/d13 | 18 | R3/d14 | 3 |
| R4/d15 | 46 | PCLK | 39 | HSYNC | 40 |
| VSYNC | 41 | DE | 42 | RESET | Board-level RC/EN path; no direct ESP32 GPIO |

**Key configuration:**

```cpp
cfg.freq_write = 18000000;
cfg.hsync_polarity = cfg.vsync_polarity = 0;
cfg.hsync_front_porch = cfg.hsync_back_porch = 8;
cfg.hsync_pulse_width = 4;
cfg.vsync_front_porch = cfg.vsync_back_porch = 8;
cfg.vsync_pulse_width = 4;
cfg.pclk_idle_high = 1;
```

- Resolution/frame buffer: `800×480`; the current SD example uses `use_psram = 2` and an RGB bus back buffer, while the other LVGL/port examples use `use_psram = 1`.
- Initialization sequence: Construct the LGFX configuration → `gfx.setColorDepth(16)` → `gfx.init()` → `gfx.initDMA()` → clear the screen. LVGL uses double buffers of 40 lines each and halts if internal RAM allocation fails.
- Parameter differences: lesson-03/05 use 16 MHz, the current lesson-04 code uses **18 MHz**, and the historical SD copy at the repository root uses 21 MHz. The maintenance baseline is the current lesson-04 value of 18 MHz. If an application is derived from lesson-03/05 and has already proven stable on hardware, 16 MHz may be retained. Any increase in PCLK requires renewed validation for flicker, tearing, and temperature rise.
- Software dependencies: LovyanGFX 1.2.25; GUI examples additionally depend on LVGL 9.1.0.

### 4.3 GT911 Capacitive Touch

- **Pins:** SDA GPIO15, SCL GPIO16; INT is physically connected to GPIO1; RST is controlled by `P1.7/TP_RST` on the STC auxiliary MCU.
- **Driver method:** I²C0 at 400 kHz, 7-bit address `0x5D`. The code also notes `0x14` as another possible address, but the startup probe specifically requires a response at `0x5D`.
- **Coordinates:** X 0~800, Y 0~480, with `offset_rotation=0`; the application can change display/touch mapping through `gfx.setRotation(2)`.
- **Interrupt:** LovyanGFX configures `pin_int=-1`; the current software uses polling and does not use the GPIO1 interrupt.
- **Reset/recovery timing:** Delay 50 ms after I²C initialization. If `0x30` and `0x5D` do not both respond, first send command 250 to `0x30`, then configure GPIO1 as an output and drive it low for 120 ms, switch it back to input, wait 100 ms, and retry.
- **Software dependencies:** LovyanGFX `Touch_GT911` + Arduino `Wire`. Do not initialize another GT911 library in parallel and allow it to contend for the same device.

### 4.4 STC8H1K28 Board Control MCU, Backlight, and Auxiliary Functions

- **Bus:** SDA GPIO15, SCL GPIO16; address `0x30` verified by the code. Commands are single-byte write transactions with no register address.
- **Known commands:** `0..245` control backlight brightness, where `0` is brightest and `245` turns it off; the value direction is opposite that of conventional PWM. `248` enables the speaker; `250` activates/recovers touch.
- **Board control connections:** The STC controls the MT9201 backlight EN/power, GT911 RST, NS4168 control path, and passive buzzer. It also reads the TP4059 `CHRG`/`DONE` signals and drives the charging/complete LEDs.
- **Key principle:** The repository does not include the STC firmware or a complete command table. Do not infer the meaning of commands other than 0~245, 248, and 250. The board control MCU address and protocol should be frozen as a product interface.

```cpp
Wire.begin(15, 16);
Wire.beginTransmission(0x30);
Wire.write(0);                 // Brightest; 245 turns it off
uint8_t status = Wire.endTransmission();
```

### 4.5 MicroSD/TF Card

- **Connections:** MISO GPIO4, SCK GPIO5, and MOSI GPIO6 connect to the card slot through a CH486F function-selection switch. Card-slot DAT1/DAT2 are pulled up, and SPI 1-bit mode is used.
- **Chip select:** The schematic net `SDCS` is not connected to an ESP32 GPIO and is fixed by a resistor. The code passes `SD_CS=0` to the SPI/SD API and explicitly comments that the “chip selector pin is not connected to IO.” Therefore, GPIO0 is a library-compatibility placeholder and cannot drive the card’s actual CS signal.
- **Clock:** The current `lesson-04` defines `SD_SPI_FREQ=40000000` (40 MHz). A historical copy at the repository root used 80 MHz, but this is not the current baseline.
- **Host:** The ESP32-S3 branch uses `SPIClass(FSPI)`, while branches for other chips use HSPI. Initialization is `SD_SPI.begin(5,4,6,0)`, followed by `SD.begin(0, SD_SPI, 40000000)`.
- **Dependencies:** Arduino `SPI`, `FS`, and `SD`; the example uses the FAT file system and displays an 800×480, 24-bit BMP with a standard 54-byte header.
- **Mutual exclusion:** Before using SD, set the CH486F DIP switches to the SD channel. In this configuration, the onboard I²S amplifier and external SPI cannot use GPIO4/5/6 simultaneously.

### 4.6 I²S Digital Amplifier and Speaker

- **Component/connections:** NS4168; GPIO4 → SDATA, GPIO5 → BCLK, and GPIO6 → LRCLK through the CH486F; differential outputs `ROUT+/-` connect to the J15 speaker connector.
- **Initialization:** After successfully probing the shared I²C bus/board controller, send `sendI2CCommand(248)` to enable the speaker, then call `audio.setPinout(5, 6, 4)`. The example uses volume 20, with a library range of 0~21.
- **Clock/sample rate:** In the example, ESP32-audioI2S configures these automatically based on the network audio stream; the code does not specify a fixed sample rate or bit width. When porting to native I²S, derive these values from the media format and test the supported range of the NS4168 on actual hardware.
- **Software dependencies:** ESP32-audioI2S 3.4.7 and Arduino WiFi/WiFiMulti; the hardware abstraction layer uses ESP-IDF I²S TX.
- **Electrical considerations:** The output is bridge-tied differential; `ROUT-` is not ground. Never connect either speaker terminal to GND, and do not directly attach the ground clip of a single-ended oscilloscope probe.

### 4.7 PDM Digital Microphone

- **Component/connections:** LMD3526B261; GPIO19 → MIC CLK, GPIO20 ← MIC DATA; the L/R selection pin is fixed by a resistor; the 3.3 V supply is filtered through a ferrite bead.
- **Driver method:** ESP32-S3 PDM RX/I²S peripheral, with GPIO19 outputting PDM CLK and GPIO20 receiving data.
- **Status:** Confirmed by the schematic, but the repository contains no recording validation program. The sample rate, clock frequency, and left/right-channel polarity all require validation on physical hardware.
- **Mutual exclusion:** GPIO19/20 also serve wireless control, UART1, and the microphone. Selection must be made through the DIP switches/CH486F, and multiple functions must not be initialized simultaneously.

### 4.8 RTC and Backup Battery

- **Component:** Schematic component value `PCF8563MDTR(XBLW)`, 32.768 kHz crystal, and CR1220 backup battery isolated through a BAT54C.
- **Connections:** SDA GPIO15 and SCL GPIO16 on the shared I²C bus.
- **Address:** The standard 7-bit address of the PCF8563 is typically `0x51`, but the repository contains no code that scans or accesses this address, so it is marked as To Be Verified.
- **Porting recommendation:** After completing board-controller/touch detection, scan `0x51`, read the seconds register, and handle the VL (low-voltage) flag. After setting the time on initial power-up, verify that timekeeping continues after main power is disconnected.
- **Note:** Only install a 3 V non-rechargeable CR1220 battery. This path is not a charging circuit.

### 4.9 Passive Buzzer

- **Connections:** The 5025 passive buzzer is driven by an NPN transistor. The control signal comes from STC `P2.7/BEEP`, with a flyback/clamping diode.
- **Driver method:** The board control MCU should output a square wave/PWM; there is no direct ESP32 GPIO connection.
- **Status:** The repository contains no buzzer command example. The control command and frequency range must be added to the STC protocol documentation. Do not experimentally write an undefined I²C byte under the assumption that it is a buzzer command.

### 4.10 External nRF24L01 Module

- **Connections:** SPI MISO4/MOSI6/SCK5; CSN GPIO19; CE GPIO20; the example does not use IRQ.
- **Parameters:** Write-pipe/read-pipe 0 address `"00001"`, RF24_PA_MAX, 250 kbps, channel 50 (2.450 GHz); the transmitter calls `stopListening()`, and the receiver calls `startListening()`.
- **Software dependency:** RF24 1.6.1; uses a separately created `SPIClass(HSPI)`.
- **Status/boundary:** This is an example for an external wireless module connected to J9/J12, not an onboard nRF24L01. GPIO19 is used as both CSN and the HSPI SS parameter, while GPIO20 is CE.
- **Power risk:** Use 3.3 V and do not connect to 5 V logic. For high-power PA/LNA modules, independently verify the 3.3 V transient-current capacity and provide local decoupling.

### 4.11 External SX1262 LoRa/LoRaWAN Module

- **Connections:** NSS GPIO8, DIO1 GPIO20, NRESET GPIO19, BUSY GPIO2, SCK5, MISO4, and MOSI6.
- **Initialization:** `SPI.begin(5,4,6,8)` → `radio.begin()` → `radio.setCurrentLimit(140.0)` → `radio.setTCXO(3.3)`.
- **Protocol:** RadioLib 7.7.1, supporting EU868/US915. The code initially constructs the node using EU868 and subBand=1, and can reconstruct the LoRaWANNode through AT parameters. It supports OTAA/ABP, ADR, DR, duty cycle, TX power, and RX2 DR.
- **Key risk:** `setTCXO(3.3)` applies only to modules with a DIO3-controlled 3.3 V TCXO. Modules using a conventional crystal or a different TCXO voltage may fail to initialize or may even be at risk of damage. The frequency band, antenna, and regional regulations must match.
- **Version note:** A code comment states “HMI Advance v1.0,” but the pins match the V1.5 expansion nets documented here. This remains an external-module configuration and does not indicate that an SX1262 is installed onboard.### 4.
12 UART, I²C, GPIO Expansion, and DHT20/Zigbee Examples

| Interface | Connections | Levels/Parameters | Notes |
|---|---|---|---|
| J2 UART0 | RXD0, TXD0, 3V3_OUT, GND | 3.3 V UART | Shares the same source as the programming/logging UART0; external devices may interfere with flashing |
| J14 Board-Control UART | STC TXD/RXD, VIN, GND | Voltage levels must be verified through measurement | Connects to the auxiliary MCU, not an ESP32 UART |
| J13 I²C | SCL16, SDA15, 3V3_OUT, GND | 3.3 V open-drain | Shared with the RTC, GT911, and STC |
| J11 GPIO | SDA15, SCL16, GPIO2, GPIO8, 3V3, GND (plus one board-control/reserved line) | 3.3 V | GPIO2/8 cannot be reused while in use by the SX1262 |
| UART1/Zigbee Example | RX19, TX20 | 115200, 8N1 | Connected to an external module through the multiplexer switch; reads by line using `\n` |
| DHT20 Example | SDA15, SCL16 | I²C; common DHT20 address: 0x38 | External Crowbits sensor with a 1 s sampling interval; no DHT20 is installed on the board |
| LED Example | GPIO19 push-pull output | High: on / Low: off | Represents an external LED only; occupies the wireless/UART/PDM multiplexing group |

The green note in the schematic requires the S1/S0 DIP switch settings to be `0/1` when using `UART1_OUT`. Because the diagram does not clearly define which physical switch positions correspond to “0/1,” verify them against the PCB silkscreen and with a multimeter continuity test during maintenance; do not infer them solely from the binary channel number.

### 4.13 USB-C, CH340K, and Programming Path

- USB-C uses only USB 2.0 D+/D-, which connect to the CH340K through series resistors; the CC pins are configured for a power sink.
- The CH340K UART connects to ESP32 UART0 through a BSS138 network; DTR/RTS control GPIO0/EN through the UMH3NTN automatic programming circuit.
- The BOOT button pulls GPIO0 low, and the RESET button pulls EN low; both are active-low.
- The debug UART defaults to 115200 baud (lesson-01 separately demonstrates 9600). Standardize the logging baud rate during porting to avoid mistaking an instructional example value for the product protocol.

### 4.14 Battery, Charging, and Power Management

- **Input path:** USB VBUS/external 5 V feeds VIN through a Schottky diode and P-MOS power path; the RY3420/HM3416H buck converter generates 3.3 V.
- **Battery charging:** The TP4059 connects to VBAT; the `CHRG` and `STD/DONE` status signals are sent to the STC and displayed by the STC/bicolor LED.
- **LCD backlight:** The MT9201 boosts VIN2 to drive two LEDA/LEDK paths, with EN controlled by the STC; this is a high-voltage constant-current node and must not be used as a GPIO test point.
- **Software boundary:** The ESP32 does not directly configure the charging current, DC-DC converter, or backlight registers; software controls only known functions through STC I²C commands.
- **To be confirmed:** The battery connector polarity, permitted capacity/protection-board requirements, and TP4059 charging current—which is determined by the hardware PRO resistor—must be verified against the BOM/physical board before being documented in the production specifications.

## 5. Recommended Initialization Sequence

1. After power-up, ensure that GPIO0, EN, and all boot strapping pins are not forcibly driven by peripherals; start `Serial` logging.
2. Run `Wire.begin(15,16)` and wait at least 50 ms.
3. Probe the board-control MCU at `0x30` and the GT911 at `0x5D`; if probing fails, execute the recovery sequence of command 250 followed by driving GPIO1 low for 120 ms. Production firmware should add a limited number of retries and fault codes to avoid an infinite loop.
4. Set the CH486F DIP switches according to the product mode, then initialize the single selected function corresponding to GPIO4/5/6 and GPIO19/20.
5. Set the backlight (0–245) or amplifier (248); the backlight may remain off until the display content is ready to reduce white-screen flashes.
6. Initialize LCD RGB/DMA and touch; allocate frame buffers after confirming that PSRAM is available.
7. Initialize selected peripherals such as SD, I²S, wireless, or UART; check the return value at each step.
8. Finally, start LVGL/application tasks; use mutexes when shared I²C, SPI, and display objects are accessed across tasks.

## 6. Schematic and Code Discrepancy Log

| ID | Discrepancy | Adopted Conclusion | Possible Cause/Handling |
|---|---|---|---|
| D-01 | The filename/directory indicates V1.5, while the large version text in the PDF title block still indicates V1.4 | Tentatively treat the document as applicable to V1.5; the physical hardware batch must be verified | The schematic title block was not updated |
| D-02 | The MCU section in the PDF states N16R8, while the device set specifies only ESP32-S3-WROOM-1 | Do not treat N16R8 as a verified fact | The EAGLE device library does not distinguish memory capacities; check the module markings and build configuration |
| D-03 | The SD code uses `SD_CS=0`, but SDCS is not connected to GPIO0 in the schematic | CS is fixed in hardware; GPIO0 is only an API placeholder | The code comments explicitly state that it is not connected; avoid accidentally driving the boot pin |
| D-04 | The root SD example uses 80 MHz, while the current lesson-04 example uses 40 MHz | Use 40 MHz | The current course code lowers the frequency to improve margin |
| D-05 | RGB PCLK is set to 16/18/21 MHz in different examples | Use 18 MHz as the general maintenance baseline; a verified application may retain 16 MHz | Differences result from example evolution/frame-buffer strategies; any change requires display stress testing |
| D-06 | The FPC color-bit names in the schematic are R3–R7/G2–G7/B3–B7, while the code comments use R0–R4/G0–G5/B0–B4 | Follow the GPIO and `d0..d15` mapping | The code numbers RGB565 logical bits, while the schematic numbers the panel’s physical 8-bit color bits; the two are not contradictory |
| D-07 | GT911 INT connects to GPIO1, but the code specifies `pin_int=-1` | The current driver uses polling; GPIO1 is reserved for recovery | The software does not use interrupts, and this line participates in the reset/address-selection recovery sequence |
| D-08 | The LoRa comments mention Advance v1.0 | Use the pins according to the current code/net connections, but identify the module as external | The example comments were not updated with the hardware documentation version |
| D-09 | The schematic does not identify the GT911 chip and provides only a touch FPC | Use the actual code driver model `Touch_GT911` | The touch controller may be located on the display/touch assembly FPC rather than in the main PCB BOM |

## 7. Pin Conflicts and Electrical Risks

### 7.1 Multiplexing Conflicts That Must Be Managed

1. **GPIO4/5/6 multiplexing group:** Select one of SD, the onboard I²S amplifier, or external SPI wireless; the physical DIP-switch settings and software initialization must match.
2. **GPIO19/20 multiplexing group:** PDM MIC, wireless CS/CE/IRQ/RST/DIO1, UART1 RX/TX, and the external LED are mutually exclusive.
3. **GPIO15/16 I²C:** The STC, GT911, RTC, and external DHT20/I²C devices share the bus; before adding a device, check for address conflicts, total pull-up resistance, and total bus capacitance.
4. **GPIO0:** This is the BOOT strapping pin and is used by the SD API as a virtual CS; external circuits must not pull it low during reset.
5. **GPIO1:** This is the touch INT/recovery line; the current code briefly drives it low in push-pull mode, so it must not be connected directly in parallel with a strongly driven output.
6. **UART0:** This interface is used simultaneously for programming, logging, and J2 expansion; peripheral output at power-up may block programming or corrupt logs.

### 7.2 Electrical and Reliability Considerations

- Treat all ESP32 GPIO and expansion signals as **3.3 V logic**; 5 V tolerance is not guaranteed. `IOT_5V`/VIN may be used only for power and must not be used as a logic-high level.
- The 21 high-speed RGB lines and PCLK should not use flying wires, and their drive strength should not be modified arbitrarily; preserve the pin matrix, polarity, and porch parameters when porting to ESP-IDF.
- The backlight boost output, LED A/K, and NS4168 differential speaker outputs are not ordinary logic nodes.
- For external RF modules, verify peak current, antenna clearance, ESD protection, and regulatory compliance; a PA_MAX or 140 mA current-limit configuration does not guarantee that the onboard 3.3 V rail has sufficient margin.
- I²C at 400 kHz is the GT911 code baseline; if the RTC or an external sensor does not support Fast-mode, reduce the shared bus to 100 kHz and revalidate touch performance.
- The current software permanently blocks in several places after hardware detection failures. Production firmware should instead use timeouts, degraded operation, error logging, and watchdog-friendly strategies.
- The network audio example in the code contains plaintext Wi-Fi credentials. Production firmware must not hard-code real credentials and should use secure provisioning/credential storage.

## 8. Software Dependencies and Porting Boundaries

| Function | Current Software Layer | Replacement When Porting to ESP-IDF/Other Frameworks |
|---|---|---|
| LCD RGB/DMA | LovyanGFX 1.2.25 | `esp_lcd_new_rgb_panel()`; copy the complete timing and GPIO tables |
| GT911 | LovyanGFX Touch_GT911 + Wire | I²C master + GT911 register driver; preserve the address/recovery sequence |
| GUI | LVGL 9.1.0 | Retain the LVGL 9 API, double buffering, and flush-ready timing |
| SD | Arduino SD/FS/SPI | SDSPI host + FATFS; the fixed CS connection requires customized/verified host behavior |
| Audio | ESP32-audioI2S 3.4.7 | ESP-IDF I²S standard/PDM API + decoder; board-control command 248 must still be retained |
| nRF24 | RF24 1.6.1 | SPI master + nRF24 register driver |
| SX1262/LoRaWAN | RadioLib 7.7.1 | Retain RadioLib or port the SX1262 HAL + LoRaWAN stack |
| Board-Control MCU | Arduino Wire single-byte write | Any I²C master; keep the protocol address/commands unchanged |
| Wi-Fi/UART/GPIO | Arduino core | ESP-IDF `esp_wifi`, UART, and GPIO drivers |

## 9. Maintenance and Porting Acceptance Checklist

- [ ] Record the physical PCB version, PDF title-block version, ESP32 module capacity marking, and display/touch assembly part number.
- [ ] Pin the Arduino-ESP32, LovyanGFX, LVGL, and other library versions, as well as build menu settings (Flash/PSRAM/partition table).
- [ ] Scan for `0x30` and `0x5D` at power-up, and verify the RTC at `0x51` if installed.
- [ ] Run solid-color, checkerboard, scrolling, and 30-minute stress tests to verify RGB order, PCLK, porch settings, tearing, and temperature rise.
- [ ] Test the four corners, edges, and multi-touch input; verify the rotation direction and GPIO1 recovery sequence.
- [ ] Verify SD, I²S, PDM, SPI wireless, and UART separately in each DIP-switch mode, confirming that no bus contention occurs.
- [ ] Repeatedly mount the SD card, read and write large files, and hot-swap it at 40 MHz; verify recovery after errors.
- [ ] Verify differential audio wiring, clipping at full volume, noise floor, and pops when enabling or disabling the amplifier.
- [ ] Test battery charging, charge-complete indication, RTC retention after power loss, low-voltage behavior, and peak system power consumption.
- [ ] Verify reliable flashing with BOOT, RESET, automatic programming, and J2 peripherals in every connection state.
- [ ] Remove example Wi-Fi passwords and keys; enable timeouts, error codes, the watchdog, and a production logging strategy.

## 10. Known Information Gaps

The following information cannot be reliably determined from the current repository and should be added to controlled documentation during subsequent maintenance: STC8H1K28 firmware source code and the complete I²C command table, the official BOM/part-substitution relationships, LCD/GT911 module datasheets, the CH486F DIP-switch truth table, validated PDM microphone parameters, battery specifications and charging current, the maximum load of each power rail, RTC software and factory time-setting procedures, and product-level EMC/ESD/RF test reports.

Until these gaps are resolved, this document may serve as a baseline for driver porting and interface maintenance, but it cannot replace production hardware specifications and certification documents.