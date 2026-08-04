file(GLOB_RECURSE AUDIO_LIB_SRCS "${CMAKE_CURRENT_LIST_DIR}/../lib/ESP32-audioI2S-3.0.12/src/*.cpp")

idf_component_register(
    SRCS
        "main.cpp"
        ${AUDIO_LIB_SRCS}

    INCLUDE_DIRS
        "."
        "../lib/ESP32-audioI2S-3.0.12/src"

    REQUIRES
        wear_levelling
        espressif__arduino-esp32
)