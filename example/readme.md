## Arduino platform requirements for ESP32 version and library version
Board Version
esp32 by Espressif System 3.0.2
After completing the Esp32 download, replace the files in that directory；
Open the "ESP32S3_120M"(https://drive.google.com/drive/u/1/folders/1npWkVzctd7e0Y6H5hahk8uxGhy9idxCz) file we provided, copy a copy of the file from that folder to the following path in ESP32, and replace it；
(C：\Users\14175\AppData\Local\Arduino15\packages\esp32\tools\esp32-arduino-libs\idf-release_v5.1-bd2b9390ef)

<img width="1267" height="541" alt="image" src="https://github.com/user-attachments/assets/10050aa2-4586-4bd2-9227-db077fdfee03" />

Note: "ESP32S3_120M" is a recompiled official file that has enabled PSRAM high-speed communication mode, and the improved refresh rate is just one of its optimizations. If not replaced, the maximum can only reach 80M, and after replacement, the maximum is 120M.
Lib Related Versions
Ivgl: 8.3.3
LovyanGFX: 1.1.16
Adafruit GFX Library ：1.11.0
The libs are provided directly by our wiki, just use the libs provided by the wiki.



# Note: The programs in the example_code_7.0 folder on this page are version 1.0 arduino programs.

# The program here is divided into different versions, according to the version of your product to choose the corresponding program

## **The basic tutorial case is divided into 1-11 lessons:**

1,Introduction to CrowPanel-Advance-HMI Screen

2,Introduce the screen user interface and external speakers for playing songs

3,Use LVGL library to create UI interface and light up lights

4,SD card stores images and displays them locally

5,Port Introduction

6,nRF2401 communication

7,wirelessmodule_lorawan

 8,Elementary chatbot（text-to-text）

 9,wirelessmodule_zigbee

 10,Build AI Chatbot

 11,LoRa Meshtastic

 



### Documentation tutorial link：

https://www.elecrow.com/pub/wiki/HMI_Display_course.html

 

### YouTube tutorial link:

https://www.youtube.com/playlist?list=PLwh4PlcPx2Gfrtm7TmlARyF4ccTmIy-gK

 

### Product purchase link:

https://www.elecrow.com/crowpanel-advance-7-0-hmi-esp32-ai-display-800x480-artificial-intelligent-ips-touch-screen-support-meshtastic-and-arduino-lvgl-micropython.html
