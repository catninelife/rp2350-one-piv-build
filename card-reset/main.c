/* EXPERIMENTAL: device-specific recovery, NOT approved for hardware use. */
#include <stdint.h>
#include "boot/picobin.h"
#include "hardware/irq.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "guard.h"

#if !PICO_NO_FLASH
#error This recovery program must execute from RAM, never from flash.
#endif

static int chip_id(uint64_t *id) {
    uint32_t data[4] = {0};
    int count = rom_get_sys_info(data, 4, SYS_INFO_CHIP_INFO);
    if (count != 4 || !(data[0] & SYS_INFO_CHIP_INFO)) return -1;
    *id = (uint64_t)data[2] | ((uint64_t)data[3] << 32);
    return 0;
}

static int partition(unsigned index, recovery_partition *out) {
    uint32_t data[3] = {0};
    uint32_t flags = PT_INFO_SINGLE_PARTITION | PT_INFO_PARTITION_LOCATION_AND_FLAGS | (index << 24);
    int count = rom_get_partition_table_info(data, 3, flags);
    if (count != 3 || !(data[0] & PT_INFO_PARTITION_LOCATION_AND_FLAGS)) return -1;
    uint32_t first = (data[1] & PICOBIN_PARTITION_LOCATION_FIRST_SECTOR_BITS) >> PICOBIN_PARTITION_LOCATION_FIRST_SECTOR_LSB;
    uint32_t last = (data[1] & PICOBIN_PARTITION_LOCATION_LAST_SECTOR_BITS) >> PICOBIN_PARTITION_LOCATION_LAST_SECTOR_LSB;
    if (last < first) return -1;
    out->start = UINT32_C(0x10000000) + first * RECOVERY_SECTOR_SIZE;
    out->end = UINT32_C(0x10000000) + (last + 1) * RECOVERY_SECTOR_SIZE;
    return 0;
}

static int erase_data(uint32_t start, uint32_t length) {
    cflash_flags_t flags = { .flags = (CFLASH_SECLEVEL_VALUE_SECURE << CFLASH_SECLEVEL_LSB) |
                                     (CFLASH_ASPACE_VALUE_STORAGE << CFLASH_ASPACE_LSB) |
                                     (CFLASH_OP_VALUE_ERASE << CFLASH_OP_LSB) };
    uint32_t irq = save_and_disable_interrupts();
    int result = rom_flash_op(flags, start, length, NULL);
    restore_interrupts(irq);
    return result;
}

int main(void) {
    uint32_t workarea[1024];
    const recovery_ops ops = {chip_id, partition, erase_data};
    if (rom_load_partition_table((uint8_t *)workarea, sizeof(workarea), true) == 0) {
        (void)recovery_run(&ops);
    }
    /* Host must check data independently; returning here is not a success signal. */
    rom_reset_usb_boot(0, 0);
    for (;;) tight_loop_contents();
}
