
file(
    GLOB_RECURSE LIB_SRC
    "../lib/ESP32-audioI2S-3.0.12/src/*.c"
    "../lib/ESP32-audioI2S-3.0.12/src/*.cpp"

)




idf_component_register(
    SRCS
    ${LIB_SRC}
    "main.cpp"
    INCLUDE_DIRS "." "../lib/ESP32-audioI2S-3.0.12/src"
    REQUIRES arduino-esp32)