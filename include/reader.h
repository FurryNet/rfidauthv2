#include "rc522.h"
#include "driver/rc522_spi.h"
#include "rc522_picc.h"
#include <picc/rc522_mifare.h>

/* Standard RC522 Reader Initializer */
void init_reader(rc522_driver_handle_t* driver, rc522_handle_t* scanner, void(*on_picc_state_changed)(void *arg, esp_event_base_t base, int32_t event_id, void *data));

/* Spaceless UID to hex implementation 
* Maximum possible length is 21 chars (2 characters/byte + 1 null term)
* uid is the reader's uid object
* uid_str is the string to be written to
* size is the actual length of the output given that it did not reach the maximum length (set null to ignore this value).
*/
void uid_to_hex(rc522_picc_uid_t uid, char* uid_str, size_t* size);

// Mifare Classic 1K/4K Card Read/Write Class
class mf_classic {
    public:
        // Constructor, given picc object and 6 bytes key
        mf_classic(rc522_handle_t * scanner, rc522_picc_t * picc, uint8_t key[6], rc522_mifare_key_type_t keyType = RC522_MIFARE_KEY_A);
        // Change current key value
        void set_key(uint8_t key[6], rc522_mifare_key_type_t keyType = RC522_MIFARE_KEY_A);

        // Read the entire block from the card (16 bytes)
        void mf_classic_read(uint8_t block, uint8_t* buffer);
        /* Read multiple blocks from a sector (16 bytes/block)
        * index - starting block index from the sector (0 = first block of the sector; up to 2 = last user-defined sector block)
        * len - Number of blocks from the sector to read (minimum 1 and up to 3 if index is 0)
        * buffer - pointer to an array of char arrays
        * Returns the actual length of the data that has been read
        */
        void mf_classic_read(uint8_t sector, uint8_t index, uint8_t len, uint8_t buffer[][16]);

        /* Write the entire block (16 bytes) of data.
        */
        void mf_classic_write(uint8_t block, uint8_t* buffer, size_t size);
        /* Write a block to the card (Up to 16 bytes).
        * index is the starting line index of the block to be written on.
        * buffer is the data to be written.
        * size is the maximum length of the data to be written
        * Returns the actual length of the data written.
        */
        size_t mf_classic_write(uint8_t block, size_t index, uint8_t* buffer, size_t size);
        
    private:
        rc522_handle_t * scanner;
        rc522_picc_t * picc;
        rc522_mifare_key_t key;
};







