/* Experimental PIV policy: use existing 9A/9D keys, deny administration.
 * Optional PIN maintenance delegates authentication and retries to PIV handlers.
 * Does not restrict other applets. PIN retry accounting remains in firmware.
 */
#ifndef YKMD_EXISTING_KEYS_H
#define YKMD_EXISTING_KEYS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static bool ykmd_existing_keys_allowed(uint8_t ins, uint8_t p1, uint8_t p2,
                                      const uint8_t *data, size_t size) {
    if (size && !data) {
        return false;
    }
    switch (ins) {
        case 0xA4: /* SELECT is also handled by the SDK dispatcher. */
            return true;
        case 0xFD: /* version */
        case 0xF8: /* serial */
            return p1 == 0 && p2 == 0 && size == 0;
        case 0x20:
            return p2 == 0x80 && ((p1 == 0 && (size == 0 || size == 8)) ||
                                   (p1 == 0xFF && size == 0));
#if defined(PICO_YKMD_ALLOW_PUK_RESET) && PICO_YKMD_ALLOW_PUK_RESET
        case 0x2C: /* RESET RETRY COUNTER: 8-byte PUK + 8-byte new PIN. */
            return p1 == 0 && p2 == 0x80 && size == 16;
#endif
#if defined(PICO_YKMD_ALLOW_PIN_CHANGE) && PICO_YKMD_ALLOW_PIN_CHANGE
        case 0x24: /* CHANGE REFERENCE DATA: old PIN + new PIN; never PUK. */
            return p1 == 0 && p2 == 0x80 && size == 16;
#endif
        case 0xF7:
            return p1 == 0 && size == 0 &&
                   (p2 == 0x9A || p2 == 0x9D || p2 == 0x9B || p2 == 0x80 || p2 == 0x81);
        case 0x87:
            return p2 == 0x9A || p2 == 0x9D; /* never management key 9B */
        case 0xCB: {
            if (p1 != 0x3F || p2 != 0xFF || (size != 3 && size != 5) ||
                data[0] != 0x5C || data[1] != size - 2) {
                return false;
            }
            uint32_t object = 0;
            for (size_t i = 2; i < size; ++i) {
                object = (object << 8) | data[i];
            }
            return object == 0x7E || object == 0x5FC101 || object == 0x5FC102 ||
                   object == 0x5FC105 || object == 0x5FC107 || object == 0x5FC10A ||
                   object == 0x5FC10B || object == 0x5FC10C ||
                   (object >= 0x5FC10D && object <= 0x5FC120) ||
                   object == 0x5FFF10 || object == 0x5FFF11;
        }
        default:
            return false;
    }
}
#endif
