#include <display.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <reader.h>
#include <string.h>

static rc522_driver_handle_t driver;
static rc522_handle_t scanner;

void arrayToHex(uint8_t* arr, size_t len, char* str) {
    for (size_t i = 0; i < len; i++) {
        sprintf(str + (i * 2), "%02X", arr[i]);
    }
    str[len * 2] = '\0'; // Null-terminate the string
}

void reader_state(void *arg, esp_event_base_t base, int32_t event_id, void *data)
{
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    if (picc->state == RC522_PICC_STATE_ACTIVE) {

        //char uid_str[RC522_PICC_UID_STR_BUFFER_SIZE_MAX];
        //rc522_picc_uid_to_str(&picc->uid, uid_str, sizeof(uid_str));
        char* name = rc522_picc_type_name(picc->type);
        unsigned short atqa = picc->atqa.source;
        unsigned char sak = picc->sak;

        // UID converter
        char uid_raw[21];
        size_t size = 0;
        uid_to_hex(picc->uid, uid_raw, &size);
        char uid_trunc[12];
        strncpy(uid_trunc, uid_raw, 12);

        // Formatted Strings
        char name_fmt[17];
        char uid_fmt[17];
        char atqa_fmt[17];
        char sak_fmt[17];
        snprintf(name_fmt, sizeof(name_fmt), "Name: %s", name);
        //snprintf(uid_fmt, picc->uid.length, "%02X", picc->uid.value);
        snprintf(uid_fmt, sizeof(uid_fmt), "UID: %s", uid_trunc);
        snprintf(atqa_fmt, sizeof(atqa_fmt), "ATQA: 0x%04X", atqa);
        snprintf(sak_fmt, sizeof(sak_fmt), "SAK: 0x%02X", sak);

        ESP_LOGI("ReaderState", "Card Detected! %s %s %s %s", name_fmt, uid_fmt, atqa_fmt, sak_fmt);
        // Write to display
        display_write_page("Card Detected!", 1);
        display_write_page(uid_fmt, 2);
        display_write_page(name, 3);
        display_write_page(atqa_fmt, 4);
        display_write_page(sak_fmt, 5);

        // Read first sector data
        uint8_t sector[3][16];
        uint8_t key[6] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
        mf_classic card(&scanner, picc, NULL);
        card.mf_classic_read(2, 0, 3, sector);

        // Format the dump
        char block1[16*2 + 1];
        char block2[16*2 + 1];
        char block3[16*2 + 1];
        // arrayToHex(sector[0], 16, block1);
        // arrayToHex(sector[1], 16, block2);
        // arrayToHex(sector[2], 16, block3);
        memcpy(block1, sector[0], 16);
        memcpy(block2, sector[1], 16);
        memcpy(block3, sector[2], 16);
        block1[16*2] = '\0';
        block2[16*2] = '\0';
        block3[16*2] = '\0';

        ESP_LOGI("ReaderState", "Classic Dump:\nBlock 0: %s\nBlock 1: %s\nBlock2: %s", block1, block2, block3);

    }
    else if (picc->state == RC522_PICC_STATE_IDLE && event->old_state >= RC522_PICC_STATE_ACTIVE) {
        ESP_LOGI("ReaderState", "Card has been removed");
        display_write_page("Card Removed!", 1);
        display_write_page("", 2);
        display_write_page("", 3);
        display_write_page("", 4);
        display_write_page("", 5);
    }
}

extern "C" {
    void app_main() {
        ESP_LOGI("H", "i2c driver installed successfully");
        i2c_setup();
        display_init();
        init_reader(&driver, &scanner, reader_state);
        display_write_page("Reader Status", 0);
        display_write_page("Waiting 4 Card!", 1);
        // while(1) {
        //     vTaskDelay(1000 / portTICK_PERIOD_MS);
        // }
    }
}