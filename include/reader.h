#include "rc522.h"
#include "driver/rc522_spi.h"
#include "rc522_picc.h"

/* Standard RC522 Reader Initializer */
void init_reader(rc522_driver_handle_t* driver, rc522_handle_t* scanner, void(*on_picc_state_changed)(void *arg, esp_event_base_t base, int32_t event_id, void *data));

/* Spaceless UID to hex implementation 
* Maximum possible length is 21 chars (2 characters/byte + 1 null term)
* uid is the reader's uid object
* uid_str is the string to be written to
* size is the actual length of the output given that it did not reach the maximum length (set null to ignore this value).
*/
void uid_to_hex(rc522_picc_uid_t uid, char* uid_str, size_t* size);