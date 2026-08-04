# 麦克风录音 5 秒后回放

本案例从提供的 `HMI-bigInch7` 工厂案例中提取音频相关代码：板载 PDM 麦克风录制 5 秒单声道、16 kHz、16 bit PCM 数据，随后通过板载 I2S 功放和扬声器回放。

## 使用方法

1. 使用支持 `ESP_I2S.h` 的 Espressif Arduino-ESP32 3.x 开发板包。
2. 在 Arduino IDE 中打开 `Mic_Record_5s_Playback.ino`，选择该 CrowPanel 对应的 ESP32-P4 板卡配置。
3. 确保 PSRAM 已启用；程序约需 160 KB 录音缓存和 320 KB 立体声播放缓存。
4. 编译并上传，打开 115200 波特率的串口监视器。
5. 上电后程序自动录音 5 秒并回放。之后在串口发送 `r` 可再次录音和回放。

## 使用的板载引脚

| 功能 | 引脚 |
| --- | --- |
| PDM 麦克风 CLK | GPIO 19 |
| PDM 麦克风 DATA | GPIO 20 |
| 扬声器 I2S BCLK | GPIO 5 |
| 扬声器 I2S LRCLK | GPIO 6 |
| 扬声器 I2S DATA | GPIO 4 |
| 音频控制 I²C SDA/SCL | GPIO 15 / GPIO 16 |

这些定义针对所提供的 CrowPanel Advance HMI 7.0 案例，不是通用 ESP32 接线。若使用外接麦克风或功放，需要按硬件修改引脚，并删除或调整地址 `0x30` 的音频控制命令。

`PLAYBACK_GAIN` 默认是 8 倍。出现破音时请减小该值；声音过小时可适当增加，但过大会导致削波失真。
