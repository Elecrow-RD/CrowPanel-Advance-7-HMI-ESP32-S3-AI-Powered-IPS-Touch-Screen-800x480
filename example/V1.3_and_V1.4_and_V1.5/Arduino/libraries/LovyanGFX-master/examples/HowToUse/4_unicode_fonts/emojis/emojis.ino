
#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>
// If autodetect fails, check `LovyanGFX/src/lgfx_user` folder to see if your device is listed and add it as follows:
// #include <lgfx_user/LGFX_3.5_RPI_LCD_A.hpp>
// If it isn't listed, see https://github.com/lovyan03/LovyanGFX/blob/master/examples/HowToUse/2_user_setting/2_user_setting.ino

#include <LGFX_Fonts.hpp> // https://github.com/tobozo/LGFX_Fonts

static LGFX lcd;

void setup()
{
  Serial.begin();
  Serial.println("Hello Emojis example");

  lcd.init();

  //// ⚠️ Loading all 64x64 emojis consumes a lot of program storage space
  //// With default partition scheme: Sketch uses 1248298 bytes (95%) of program storage space. Maximum is 1310720 bytes.
  lgfx::LGFX_Emojis::loadEmojis(&lcd, all_groups_64x64);

  //// Loading all emojis consumes a significant part of program storage space
  // lgfx::LGFX_Emojis::loadEmojis(&lcd, all_groups_24x24);

  //// Loading all emojis from a smaller set may save some of it
  // lgfx::LGFX_Emojis::loadEmojis(&lcd, all_groups_16x16);

  //// Loading all emojis from an even smaller set will produce pixelated emojis
  // lgfx::LGFX_Emojis::loadEmojis(&lcd, all_groups_8x8);

  // Mixing different sizes is okay, just create a custom set from subgroups
  // const emoji_png_set_t emoji_subgroups[] = {
  //   subgroup_FACE_SMILING_32x32,
  //   subgroup_FACE_TONGUE_8x8,
  //   subgroup_HAND_FINGERS_OPEN_24x24,
  //   subgroup_TRANSPORT_SIGN_64x64
  // };
  // lgfx::LGFX_Emojis::loadEmojis(&lcd, emoji_subgroups);

  //// Or create bigger custom sets from groups
  // const emoji_png_group_t emoji_groups[] = {
  //   group_SMILEYS_EMOTION_64x64,
  //   group_PEOPLE_BODY_64x64
  // };
  // lgfx::LGFX_Emojis::loadEmojis(&lcd, emoji_groups);


  lcd.fillScreen(0xddddddu);

  lcd.setTextColor(TFT_BLUE, TFT_BLACK);
  lcd.setTextSize(4.5);
  lcd.println();

  lcd.println("abc😁def");

#if defined M5GFX_LVGL_INTERNAL_H
  lcd.setFont(&fonts::lv_font_montserrat_28);
#else
  lcd.setFont(&fonts::lvFontMontserrat28);
#endif

  lcd.setTextSize(2);

  lcd.printf("%s\n", "+😋-");

  lcd.drawString("'🖖'", 100, 100);

  delay(5000);

  lcd.setTextSize(1);
  lcd.setCursor(0,0);
}


void loop()
{
  lcd.setTextColor(random(0x10000)| 0x8410, random(0x10000)&0x7BEF);

  auto emojis = lgfx::LGFX_Emojis::emojis();
  for(int i=0;i<lgfx::LGFX_Emojis::count();i++)
  {
    auto item = emojis[i].ptr;

    if(lcd.getCursorY()+lcd.fontHeight()/2>lcd.height()) {
      lcd.setCursor(0,0);
    }
    lcd.print( item->emoji );
  }
}
