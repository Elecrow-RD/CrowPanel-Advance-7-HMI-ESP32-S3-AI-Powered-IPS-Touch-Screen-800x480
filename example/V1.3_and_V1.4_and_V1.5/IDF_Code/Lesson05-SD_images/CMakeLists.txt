# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(SD_Display_Images_70_50_43)

# Work around an xtensa-esp-elf-gcc 14.2 optimizer ICE in the IDF 5.5.4
# RGB panel driver without changing the ESP-IDF component source.
idf_component_get_property(esp_lcd_lib esp_lcd COMPONENT_LIB)
set_source_files_properties(
    "$ENV{IDF_PATH}/components/esp_lcd/rgb/esp_lcd_panel_rgb.c"
    TARGET_DIRECTORY ${esp_lcd_lib}
    PROPERTIES COMPILE_OPTIONS "-O0"
)