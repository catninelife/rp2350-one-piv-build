/* Non-mutating RAM startup and partition-table probes. */
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "boot/picoboot_constants.h"

#if !PICO_NO_FLASH
#error Probe must be a RAM image.
#endif
#ifndef PROBE_CHECK_PARTITION
#error Probe stage must be explicit.
#endif

int main(void) {
    uint32_t marker = 5;
#if PROBE_CHECK_PARTITION
    static uint32_t workspace[1024];
    int rc = rom_load_partition_table((uint8_t *)workspace, sizeof(workspace), true);
    marker = rc == 0 ? 6u : 7u;
#endif
    (void)rom_reboot(REBOOT2_FLAG_REBOOT_TYPE_BOOTSEL | REBOOT2_FLAG_NO_RETURN_ON_SUCCESS,
                     10, marker, 0);
    rom_reset_usb_boot(0, 0);
    for (;;) tight_loop_contents();
}
