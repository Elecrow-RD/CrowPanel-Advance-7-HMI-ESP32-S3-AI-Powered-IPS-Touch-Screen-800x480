# 3. Use SquareLine Studio and LVGL libraries to create a UI interface to light the lights

## **1.** Introduce LVGL

------

LVGL (LittlevGL) is an open-source, lightweight, high-performance embedded graphics library designed specifically for devices with limited resources. It supports multi platform porting, provides rich controls, animations, touch support, and highly customizable styles, suitable for fields such as smart homes, industrial equipment, medical instruments, etc. LVGL is centered around modular design and can run on bare metal or operating systems, accelerating GUI development through powerful community support and tools such as SquareLine Studio.

![1](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/1.webp)

SquareLine Studio is a next-generation user interface (UI) solution for individuals and professionals, allowing users to quickly and easily design and develop aesthetically pleasing UI for embedded devices. This software provides integrated design, prototyping, and development capabilities, supporting the export of platform independent C or MicroPython code for LVGL (Lightweight Graphics Library), which can be compiled and run on any vendor's device.

![2](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/2.webp)

## 2.Install SquareLine Studio

------

**2.1** Installation Guide

Enter the https://squareline.io/ to download the SquareLine installation file.

![3](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/3.webp)



Download the version 1.4.0

![4](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/4.webp)

Double-click the setup.exe file.

![5](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/5.webp)

Click install.

![6](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/6.webp)

Wait for installation.

![7](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/7.webp)

Installation finish.

![8](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/8.webp)

There is a 30-day trial period for the first time you use it. Please follow the prompts to register an account. You will continue to use it when you log in to your account next time.

![9](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/9.webp)

### **2.2** Software Function Introduction

Open the software

The historical project page: open the project built earlier.

![10](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/10.webp)

Create a project page: choose different platforms according to different hardware of the project.

![11](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/11.webp)

###### When select the Arduino framwork, there's only one option "Arduino with TFT_eSPI". By choosing this, the squareline studio will generate a template code suitable for the TFT_eSPI library. However, squareline studio not only supports the TFT_eSPI library, it supports a variety of libraries to suit different hardware and application needs. For example, Adafruit_GFX library, LovyanGFX etc.

###### After using SLS to generate UI code, we then use different graphics libraries according to different hardware and modify the corresponding code to display the content you design.

Example page. This page has several official examples for reference.

![12](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/12.webp)

The project settings bar is used to make basic settings for the project, including property settings such as project name, screen size, display angle, etc.

![13](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/13.webp)

###### Note： Please select the corresponding resolution and color depth according to the screen specifications.

![14](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/14.webp)

##### ①Toolbar, including File, Export, and Help. Basic file operation bar, create or open files, export UI files and other operations. Click help and there are related introductory documents.

##### ②Screen bar, the project screen will be listed here.

##### ③Widget area, all widgets are here and can be selected and used according to project needs.

##### ④Hierarchy area, it will show every widget used in each screen.

##### ⑤This area shows the actual display effect, you can adjust the widgets or screen here.

##### ⑥Material column, the added materials are displayed here.

##### ⑦Setting bar, where you can make basic settings for each part, including the basic attributes and trigger operations of the part.

##### ⑧Theme bar, different themes can be set.

## 3 Use SquareLine Studio to create UI control interfaces

------

Firstly, open the SquareLine Studio software and create a case study

![15](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/15.webp)

Choose the correct resolution based on the different sizes you are using

![16](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/16.webp)

Here, I take a 7.0-inch screen as an example. After determining the resolution, I fill it in

![17](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/17.webp)

After selecting the parameters, click Create

![18](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/18.webp)

Open the photo of the desk lamp we provided and add it in. (Of course, you can also choose the image you want to use)

![19](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/19.webp)

![20](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/20.webp)

The lamp_28 image size is available for both 2.4-inch and 2.8-inch screens.

Table_1amp image sizes are available for 3.5-inch, 4.3-inch, 5.0-inch, and 7-inch screens.

![21](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/21.webp)

After adding it, drag and drop the image in.

![22](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/22.webp)

The task we need to complete is to turn on and off the lights by clicking on the buttons on the graphical interface. So we need to design two buttons

From the left sidebar, select Button and drag it into the interface.

![23](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/23.webp)

You can adjust the border of the button with the mouse, which can adjust the size of the button, and you can also drag and drop the button to adjust its position.

Then, by selecting Background, choose the background color of the Button.

![24](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/24.webp)

![25](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/25.webp)

We have made the button patterns, and now we need to add labels to the button patterns in order to distinguish their functions.

Drag and drop from the left sidebar, select Label, and drag into the interface.

![26](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/26.webp)

Modify the text content of the label

![27](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/27.webp)

And modify the font size and text color of the text

![28](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/28.webp)

The Button and Label have been designed. Click on the Hierarchy in the right-hand column and drag the Label onto the Button line to merge them into one.

![29](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/29.webp)

At this point, if you drag the buttons again, you will find that they are dragged together

Next, we will copy a completed button, right-click, and paste it in.

![30](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/30.webp)

![31](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/31.webp)

![32](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/32.webp)

Click the second button ON to change the text content to Off. Used to achieve the effect of turning off lights.

![33](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/33.webp)

## **4** Add functions to the buttons to enable them to turn on and off lights

------

Select 'event' to add in the right sidebar

![34](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/34.webp)

Select 'released' as the triggering condition and 'Call function' as the action.

![35](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/35.webp)

After selecting, click ADD.

And add a function name to the CALL Function. (Customization is sufficient)

![36](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/36.webp)

Similarly, add an event to the Off button. The operation process is the same as above.

![37](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/37.webp)

After adding, run

![38](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/38.webp)

![39](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/39.webp)

## **5** UI interface design completed, exporting UI files for easy use in subsequent code

------

Click on Project Setting

![40](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/40.webp)

Set export path

![41](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/41.webp)

![42](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/42.webp)

Complete the setup and export file

![43](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/43.webp)

Copy and paste the exported code into the code folder we need to open

![44](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/44.webp)

![45](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/45.webp)

Then double-click BigInch_LVGL.ino and open it using Arduino IDE


## **6 Connect the light bulb and add code to control the light bulb to turn on and off**

------

Connect the light bulb at the UART1 interface

![46](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/46.webp)

Observing the schematic diagram of this size, it is known that the pin for UART1 to control the light bulb to turn on and off is 19

![47](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/47.webp)

![48](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/48.webp)

A large-sized screen requires setting 19 pins as the output mode.

(Due to the fact that pin 19 in the circuit of large-sized products controls both the wireless module and the UART1 interface, it is necessary to switch the mode to 0 1)

![49](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/49.webp)

![SquareLine1](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/SquareLine1.webp)

And add the function of turning on and off the light bulb in the ui_ event. c file.

(When the On button is clicked, the light is on; when the Off button is clicked, the light is off)

![50](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/50.webp)


### *Kind Reminder:*

You are currently viewing the 7-inch product of CrowPanel Advance HMI, and the version here is V1.3.

In terms of hardware, we use a microcontroller (STC8H1K28) to control the screen backlight, speaker on/off, and buzzer.

![advance-7-1.3-1](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/advance-7-1.3-1.webp)

(However, there are other function interfaces that need to be written in the specific code, and you can refer to the complete code provided later.)

Explanation:

- 0x30 is the I2C address of the microcontroller (STC8H1K28).
- 0x5D is the I2C address of the touch IC (GT911).
- sendI2CCommand(0) means sending command 0 to the microcontroller (address 0x30) to instruct it to set the screen brightness to maximum.

For 0 mentioned above, you can replace it with the following values:

- 0 is the brightest backlight.    
- 0 to 245: The screen brightness will gradually increase to the minimum value
- 245 represents turning off the screen light

Additional notes:

You can also control the following functions by sending other instructions to the microcontroller:

- It means sending the 248 command to the microcontroller (0x30) to instruct the speaker to turn on.

![advance-7-1.3-2](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/advance-7-1.3-2.webp)

- It means sending the 249 command to the microcontroller (0x30) to instruct the speaker to turn off.

![advance-7-1.3-3](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/advance-7-1.3-3.webp)

- You can also send command 246 to control the buzzer to turn on, and send command 247 to control the buzzer to turn off.

![advance-7-1.3-4](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/advance-7-1.3-4.webp)




## **7 Configure the code running environment and upload the code**

------

(!! Before uploading the code, please use different library files according to the size you are using. You can review the content of Lesson 2)

(For large-sized screens of 4.3 inches, 5.0 inches, and 7.0 inches, be sure to switch the mode to 0 1 state before uploading the code, because the pins of UART1 and W-M mode are both used in 0 1 mode, so that you can use the pins of UART1)

![51](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/51.webp)

![52](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/52.webp)

So you can see the UI interface you just designed

![53](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/53.webp)

You can click the On and Off buttons to control the on/off of the lights

![54](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/54.webp)

![55](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/55.webp)

## 8 Code presentation

------

### 7.0 inches, 5.0 inches, 4.3 inches

```c
#include "pins_config.h"
#include "LovyanGFX_Driver.h"

#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <SPI.h>

#include <stdbool.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "ui.h"

LGFX gfx;

/* Change to your screen resolution */
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf;
static lv_color_t *buf1;

uint16_t touch_x, touch_y;

//  Display refresh
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  if (gfx.getStartCount() > 0) {
    gfx.endWrite();
  }
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);

  lv_disp_flush_ready(disp);  //	Tell lvgl that the refresh is complete
}

//  Read touch
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
  data->state = LV_INDEV_STATE_REL;// The state of data existence when releasing the finger
  bool touched = gfx.getTouch( &touch_x, &touch_y );
  if (touched)
  {
    data->state = LV_INDEV_STATE_PR;

    //  Set coordinates
    data->point.x = touch_x;
    data->point.y = touch_y;
  }
}

bool i2cScanForAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

// Wrapper function for sending I2C commands
void sendI2CCommand(uint8_t command) {
  uint8_t error;
  // Start sending commands to the specified address
  Wire.beginTransmission(0x30);
  // Send command
  Wire.write(command);
  //  End transmission and return status
  error = Wire.endTransmission();

  if (error == 0) {
    Serial.print("command 0x");
    Serial.print(command, HEX);
    Serial.println(" Sent successfully");
  } else {
    Serial.print("Command sent error, error code:");
    Serial.println(error);
  }
}

void setup()
{
  Serial.begin(115200); 

  pinMode(19, OUTPUT);

  Wire.begin(15, 16);
  delay(50);
  while (1) {
    if (i2cScanForAddress(0x30) && i2cScanForAddress(0x5D)) {
      Serial.print("The microcontroller is detected: address 0x");
      Serial.println(0x30, HEX);
      Serial.print("The microcontroller is detected: address 0x");
      Serial.println(0x5D, HEX);
      break;
    } else {
      Serial.print("No microcontroller was detected: address 0x");
      Serial.println(0x30, HEX);
      Serial.print("No microcontroller was detected: address 0x");
      Serial.println(0x5D, HEX);
      //Prevent the microcontroller did not start to adjust the bright screen
      sendI2CCommand(250);    // 250 : Activate touch screen
      pinMode(1, OUTPUT);
      digitalWrite(1, LOW);
      //ioex.output(2, TCA9534::Level::L);
      //ioex.output(2, TCA9534::Level::H);
      delay(120);
      pinMode(1, INPUT);

      delay(100);
    }
  }
  // Start sending command 0 to address 0x30
  sendI2CCommand(0);  // 0 is the brightest backlight.    / 245 backlight off   (0-245)

  // Init Display
  gfx.init();
  gfx.initDMA();
  gfx.startWrite();
  gfx.fillScreen(TFT_BLACK);

  lv_init();
  size_t buffer_size = sizeof(lv_color_t) * LCD_H_RES * LCD_V_RES;
  buf = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  buf1 = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);

  lv_disp_draw_buf_init(&draw_buf, buf, buf1, LCD_H_RES * LCD_V_RES);

  // Initialize display
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  // Change the following lines to your display resolution
  disp_drv.hor_res = LCD_H_RES;
  disp_drv.ver_res = LCD_V_RES;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize input device driver program
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  delay(100);
  gfx.fillScreen(TFT_BLACK);
  // lv_demo_widgets();// Main UI interface
  ui_init();

  Serial.println( "Setup done" );
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(1);
}

```

**If your code compiles incorrectly, you can check if the ESP32 version number is correct. The ESP32 version number we need for this lesson is 3.0.2.**

![new-1](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/new-1.webp)

**Secondly, please pay attention to replacing the corresponding size library file.**

**Select the appropriate library file based on the product screen size**

Path reference：C:\ESP32_Code\CrowPanel_Advanced_HMI\Arduino_lib Series Library

![new-2](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/new-2.webp)

**I will use the Advance 7.0-inch product as an example for operation**

Copy the Libraries Advanced 7.0 folder

Open Arduino IDE runtime library file path

Reference path: C:\Users\14175\Documents\Arduino

![new-3](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/new-3.webp)

Delete the existing libraries folder

![new-4](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/new-4.webp)

Paste the copied library Advanced 7.0 folder into this path

![new-5](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/new-5.webp)

Change the folder name to the original libraries

![new-6](https://www.elecrow.com/wiki/assets/images/5.0_3_Use_LVGL_library_to_create_UI_interface_and_light_up_lights/new-6.webp)

Library update completed, restart Arduino IDE.

**When using other sizes, changing the library file is the same operation**

## 5inch and 7 inch  Key Points.

------

**5 inch Version 1.1** and **7 inch Version 1.2** updated the control of backlight on the basis of the original, the backlight of version 1.1 is controlled by STC8H1K28 microcontroller, and the backlight is lit in the program by sending the value to this microcontroller address (0x30). The values are 0x05, 0x06, 0x07, 0x08, 0x09 and 0x10, where 0x05 switches off the backlight and 0x10 is the maximum brightness.

**To set a different brightness level, you must first send 0x10 to turn on the screen, and then send another value to adjust the brightness.**

The buzzer and speaker operate on the same principle as the backlight control: sending 0x15 activates the buzzer, while sending 0x16 turns it off. Sending 0x17 turns the speaker on, and sending 0x18 turns it off.


**4.3 inch  Version 1.1** 、**5 inch Version 1.2** and **7 inch Version 1.3**

- 0x30 is the I2C address of the microcontroller (STC8H1K28).
- 0x5D is the I2C address of the touch IC (GT911).
- sendI2CCommand(0) means sending command 0 to the microcontroller (address 0x30) to instruct it to set the screen brightness to maximum.

For 0 mentioned above, you can replace it with the following values:

- 0 is the brightest backlight.
- 0 to 245: The screen brightness will gradually increase to the minimum value
- 245 represents turning off the screen light

Additional notes:

You can also control the following functions by sending other instructions to the microcontroller:

- It means sending the 248 command to the microcontroller (0x30) to instruct the speaker to turn on.
