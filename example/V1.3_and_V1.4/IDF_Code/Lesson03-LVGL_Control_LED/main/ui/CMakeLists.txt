SET(SOURCES screens/ui_Screen1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_table_lamp_png.c
    images/ui_img_lamp_28_png.c)

add_library(ui ${SOURCES})
