SET(SOURCES components/ui_comp_startmeeting.c
    components/ui_comp.c
    screens/ui_Screen1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_background_big_png.c
    images/ui_img_lamp_28_png.c)

add_library(ui ${SOURCES})
