#include <reader.h>
#include <string.h>

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

// Mifare classic custom class below

mf_classic::mf_classic(rc522_handle_t * scanner, rc522_picc_t * picc, uint8_t key[6], rc522_mifare_key_type_t keyType) {
    if(!rc522_mifare_type_is_classic_compatible(picc->type))
        return;

    this->set_key(key, keyType);
    this->scanner = scanner;
    this->picc = picc;
}
void mf_classic::set_key(uint8_t key[6], rc522_mifare_key_type_t keyType) {
    this->key.type = keyType;
    if(key == NULL)
        return (void)memset(this->key.value, 0xFF, 6); // RC522_MIFARE_KEY_VALUE_DEFAULT
    memcpy(this->key.value, key, 6);
}

void mf_classic::mf_classic_read(uint8_t block, uint8_t* buffer) {
    rc522_mifare_auth(*this->scanner, picc, block, &this->key);
    rc522_mifare_read(*this->scanner, this->picc, block, buffer);
    rc522_mifare_deauth(*this->scanner, this->picc);
}

void mf_classic::mf_classic_read(uint8_t sector, uint8_t index, uint8_t len, uint8_t buffer[][16]) {
    // General sanity check
    if(index > 3 || len > 3 || index + len > 3 || len == 0 || buffer == NULL)
        return;

    // Authenticate the sector
    uint8_t first_sector_block = 0;
    rc522_mifare_get_sector_block_0_address(sector, &first_sector_block);
    rc522_mifare_auth(*this->scanner, this->picc, first_sector_block, &this->key);

    for(int i=index; i<index+len; i++) {
        uint8_t block = first_sector_block + i;
        rc522_mifare_read(*this->scanner, this->picc, block, buffer[i]);
    }

    rc522_mifare_deauth(*this->scanner, this->picc);
    

}

void mf_classic::mf_classic_write(uint8_t block, uint8_t* buffer, size_t size) {

}

size_t mf_classic::mf_classic_write(uint8_t block, size_t index, uint8_t* buffer, size_t size) {
    return 0;
}