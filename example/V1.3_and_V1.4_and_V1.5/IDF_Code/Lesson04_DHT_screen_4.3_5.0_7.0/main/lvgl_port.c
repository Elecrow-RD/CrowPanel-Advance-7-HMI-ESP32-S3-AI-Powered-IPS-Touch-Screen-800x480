#include "lvgl_port.h"

#include <assert.h>

#include "esp_heap_caps.h"
#include "esp_check.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char *TAG = "lvgl_port";
static SemaphoreHandle_t lvgl_mutex;

static void display_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixel_map)
{
    esp_lcd_panel_handle_t panel = lv_display_get_user_data(display);
    esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, pixel_map);
    lv_display_flush_ready(display);
}

static lv_display_t *display_init(esp_lcd_panel_handle_t panel)
{
    const size_t buffer_pixels = LVGL_PORT_H_RES * LVGL_PORT_BUFFER_HEIGHT;
    const size_t buffer_bytes = buffer_pixels * sizeof(lv_color16_t);
    void *buffer = heap_caps_malloc(buffer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (buffer == NULL) return NULL;
    lv_display_t *display = lv_display_create(LVGL_PORT_H_RES, LVGL_PORT_V_RES);
    lv_display_set_user_data(display, panel);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, buffer, NULL, buffer_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, display_flush);
    return display;
}

static void touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    esp_lcd_touch_handle_t touch = lv_indev_get_user_data(indev);
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t points = 0;
    esp_lcd_touch_read_data(touch);
    if (esp_lcd_touch_get_coordinates(touch, &x, &y, NULL, &points, 1) && points > 0) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void tick_increment(void *arg) { lv_tick_inc(LVGL_PORT_TICK_PERIOD_MS); }

static void lvgl_task(void *arg)
{
    uint32_t delay_ms = LVGL_PORT_TASK_MIN_DELAY_MS;
    while (true) {
        if (lvgl_port_lock(-1)) {
            delay_ms = lv_timer_handler();
            lvgl_port_unlock();
        }
        delay_ms = delay_ms < LVGL_PORT_TASK_MIN_DELAY_MS ? LVGL_PORT_TASK_MIN_DELAY_MS : delay_ms;
        delay_ms = delay_ms > LVGL_PORT_TASK_MAX_DELAY_MS ? LVGL_PORT_TASK_MAX_DELAY_MS : delay_ms;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

esp_err_t lvgl_port_init(esp_lcd_panel_handle_t panel, esp_lcd_touch_handle_t touch)
{
    lv_init();
    lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    ESP_RETURN_ON_FALSE(lvgl_mutex != NULL, ESP_ERR_NO_MEM, TAG, "create LVGL mutex");
    const esp_timer_create_args_t args = {.callback = tick_increment, .name = "lvgl_tick"};
    esp_timer_handle_t timer;
    ESP_RETURN_ON_ERROR(esp_timer_create(&args, &timer), TAG, "create tick timer");
    ESP_RETURN_ON_ERROR(esp_timer_start_periodic(timer, LVGL_PORT_TICK_PERIOD_MS * 1000), TAG, "start tick timer");
    ESP_RETURN_ON_FALSE(display_init(panel) != NULL, ESP_ERR_NO_MEM, TAG, "initialize display");
    if (touch != NULL) {
        lv_indev_t *indev = lv_indev_create();
        lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev, touch_read);
        lv_indev_set_user_data(indev, touch);
    }
    BaseType_t core = LVGL_PORT_TASK_CORE < 0 ? tskNO_AFFINITY : LVGL_PORT_TASK_CORE;
    ESP_RETURN_ON_FALSE(xTaskCreatePinnedToCore(lvgl_task, "lvgl", LVGL_PORT_TASK_STACK_SIZE, NULL,
                                               LVGL_PORT_TASK_PRIORITY, NULL, core) == pdPASS,
                        ESP_ERR_NO_MEM, TAG, "create LVGL task");
    return ESP_OK;
}

bool lvgl_port_lock(int timeout_ms)
{
    assert(lvgl_mutex != NULL);
    return xSemaphoreTakeRecursive(lvgl_mutex, timeout_ms < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void lvgl_port_unlock(void) { xSemaphoreGiveRecursive(lvgl_mutex); }
bool lvgl_port_notify_rgb_vsync(void) { return false; }
