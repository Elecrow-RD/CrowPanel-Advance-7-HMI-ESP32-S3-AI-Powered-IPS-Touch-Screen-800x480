idf_component_register(
                    SRCS 
                        "SD_Display_Images_70_50_43.cpp"
                    INCLUDE_DIRS
                        "."
                    REQUIRES
                        arduino-esp32
                        LovyanGFX-master
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LGFX_USE_V1)
