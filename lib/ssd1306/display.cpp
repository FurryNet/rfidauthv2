#include <display.h>
#include <string.h>
#include "driver/i2c.h"
#include <ssd1306.h>
#include <font8x8_basic.h>
#include <esp_log.h>
#include <math.h>
#include <freertos/queue.h>

#define SDA_PIN 6
#define SCL_PIN 7
#define frequency 1000000 // 1MHz
#define SSD1306_I2C_ADDRESS 0x3C

#define TAG "display"

SSD1306_t dev;
QueueHandle_t writePageQueue = xQueueCreate(32, sizeof(displayQueue_t));

// Initalize the display pins
void display_clear();
void display_write_queue(void *pvParameters);

// General I2C Initialization
void i2c_setup() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = SCL_PIN;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = frequency;
    conf.clk_flags = 0;
    i2c_param_config(I2C_NUM_0, &conf);
    if(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0) == ESP_OK)
        ESP_LOGI(TAG, "i2c driver installed successfully");
    else
        ESP_LOGE(TAG, "i2c driver failed to install");
}

void display_init()
{
	dev._address = 0x3C;
	dev._flip = false;
	ssd1306_init(&dev, 256, 64);
	display_clear();
	xTaskCreate(display_write_queue, "display_write_queue", configMINIMAL_STACK_SIZE-512, NULL, 5, NULL);
}

// Clean the display
void display_clear() {
	xQueueReset(writePageQueue);
	ssd1306_clear_screen(&dev, false);
}

// Render text on the display
void display_text(const char* text) {
	uint8_t text_len = strlen(text);
    ssd1306_display_text(&dev, 0, const_cast<char*>(text), text_len, false);
}

// Write text to a specific line on the display (isCenter is used to center the text on the line)
void display_write_page(const char* text, int page, bool isCenter) {
	// Get the length and allocate the space
	size_t text_len = strlen(text); // Each line only supports 16 characters
	char* strArr = (char*)malloc(sizeof(char)*17);
	text_len = text_len > 16 ? 16 : text_len;

	// Check if the text needs to be centered
	if(isCenter) {
		int padLen = ceil((16 - text_len) / 2);
		for(int i = 0; i < padLen; i++)
			strArr[i] = ' ';
		for(int i = padLen; i < padLen+text_len; i++)
			strArr[i] = text[i - padLen];
		text_len+=padLen;

		// Fill in the rest of the string with byte 0
		for(int i = text_len; i < 16; i++)
			strArr[i] = '\0';
	} else strncpy(strArr, text, 16);

	// Add it to the queue
	displayQueue_t data;
	data.text = strArr;
	data.page = page;
	xQueueSend(writePageQueue, &data, 0);
}

/*
Internal function to handle writeLineQueue
Executing ssd1306 display command simultaneously causes the display to glitch out
*/
void display_write_queue(void *pvParameters) {
	while(1) {
		displayQueue_t data;
		if(xQueueReceive(writePageQueue, &data, portMAX_DELAY) == pdTRUE) {
			/* Handler Stuff Here */
			ssd1306_display_text(&dev, data.page, data.text, 16, false);
			free(data.text);
		}
		else
			vTaskDelay(pdMS_TO_TICKS(5));
	}
}