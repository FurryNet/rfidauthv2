#include <display.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
extern "C" {
    void app_main() {
        ESP_LOGI("H", "i2c driver installed successfully");
        i2c_setup();
        display_init();
        display_write_page("HEllo WoRlD", 0);
        // while(1) {
        //     vTaskDelay(1000 / portTICK_PERIOD_MS);
        // }
    }
}