#include <reader.h>

#define sda_pin 6
#define scl_pin 7
#define miso_pin 2
#define mosi_pin 10
#define sck_pin 11
#define cs_pin 4
#define rst_pin GPIO_NUM_3 // 3
#define frequency 100000 // instead of 400000



void init_reader(rc522_driver_handle_t* driver, rc522_handle_t* scanner, void(*on_picc_state_changed)(void *arg, esp_event_base_t base, int32_t event_id, void *data)) {
    spi_bus_config_t spi_conf = {
        .mosi_io_num = mosi_pin,
        .miso_io_num = miso_pin,
        .sclk_io_num = sck_pin,
    };

    static rc522_spi_config_t driver_config = {
        .host_id = SPI2_HOST,
        .bus_config = &spi_conf,
        .dev_config = {
            .spics_io_num = cs_pin,
        },
        .rst_io_num = GPIO_NUM_NC,
    };

    rc522_spi_create(&driver_config, driver);
    rc522_driver_install(*driver);

    rc522_config_t scanner_config = {
        .driver = *driver,
    };
    rc522_create(&scanner_config, scanner);
    if(on_picc_state_changed != NULL)
        rc522_register_events(*scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed, NULL);
    rc522_start(*scanner);
}

void uid_to_hex(rc522_picc_uid_t uid, char* uid_str, size_t* size){
    for(int i=0;i<uid.length;i++)
        snprintf(&uid_str[i*2], 3,"%02X", uid.value[i]);
    if(size != NULL)
        *size = uid.length*2 + 1;
};