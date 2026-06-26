# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(lesson_2-open_http_mp3_big)

add_compile_definitions(BOARD_HAS_1MSRAM)

# Workaround: xtensa-esp-elf-gcc 14.2 ICE in esp_lcd_panel_rgb.c (RTL pass: ira)
idf_component_get_property(esp_lcd_lib esp_lcd COMPONENT_LIB)
set_source_files_properties(
    "$ENV{IDF_PATH}/components/esp_lcd/rgb/esp_lcd_panel_rgb.c"
    TARGET_DIRECTORY ${esp_lcd_lib}
    PROPERTIES COMPILE_OPTIONS "-O0"
)