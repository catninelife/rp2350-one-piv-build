#include <assert.h>
#include <stdio.h>
#include "ykmd_existing_keys.h"

int main(void) {
    const uint8_t cert[] = {0x5C, 3, 0x5F, 0xC1, 0x0B};
    const uint8_t admin[] = {0x5C, 3, 0x5F, 0xFF, 0};
    const uint8_t printed[] = {0x5C, 3, 0x5F, 0xC1, 9};
    const uint8_t pin[8] = {0}; /* Synthetic only. */
    assert(ykmd_existing_keys_allowed(0x20, 0, 0x80, pin, sizeof(pin)));
    assert(ykmd_existing_keys_allowed(0x20, 0, 0x80, NULL, 0));
    assert(ykmd_existing_keys_allowed(0x20, 0xFF, 0x80, NULL, 0));
    assert(!ykmd_existing_keys_allowed(0x20, 0xFF, 0x80, pin, 8));
    assert(!ykmd_existing_keys_allowed(0x20, 0, 0x81, pin, 8));
    assert(!ykmd_existing_keys_allowed(0x20, 0, 0x80, pin, 7));
    assert(!ykmd_existing_keys_allowed(0x20, 0, 0x80, NULL, 8));
    assert(ykmd_existing_keys_allowed(0x87, 7, 0x9A, pin, 8));
    assert(ykmd_existing_keys_allowed(0x87, 0x11, 0x9D, pin, 8));
    for (unsigned p1 = 0; p1 < 256; ++p1) {
        assert(!ykmd_existing_keys_allowed(0x87, (uint8_t)p1, 0x9B, pin, 8));
    }
    const uint8_t writes[] = {0x24, 0x2C, 0x47, 0xDB, 0xFF, 0xF6, 0xFA, 0xFB, 0xFE, 0xF2};
    for (size_t i = 0; i < sizeof(writes); ++i) {
        for (unsigned p2 = 0; p2 < 256; ++p2) {
            assert(!ykmd_existing_keys_allowed(writes[i], 0, (uint8_t)p2, pin, 8));
        }
    }
    assert(ykmd_existing_keys_allowed(0xCB, 0x3F, 0xFF, cert, sizeof(cert)));
    assert(!ykmd_existing_keys_allowed(0xCB, 0x3F, 0xFF, admin, sizeof(admin)));
    assert(!ykmd_existing_keys_allowed(0xCB, 0x3F, 0xFF, printed, sizeof(printed)));
    for (size_t size = 0; size < sizeof(cert); ++size) {
        assert(!ykmd_existing_keys_allowed(0xCB, 0x3F, 0xFF, cert, size));
    }
    assert(!ykmd_existing_keys_allowed(0xCB, 0x3F, 0xFF, NULL, sizeof(cert)));
    puts("existing-key policy tests passed (including 2,560 mutation cases)");
    return 0;
}
