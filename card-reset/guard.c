#include "guard.h"

_Static_assert((RECOVERY_DATA_START % RECOVERY_SECTOR_SIZE) == 0, "unaligned start");
_Static_assert((RECOVERY_DATA_END % RECOVERY_SECTOR_SIZE) == 0, "unaligned end");
_Static_assert(RECOVERY_DATA_END > RECOVERY_DATA_START, "empty data region");
/* The card's separate binding region ends at 0x10102000. */
_Static_assert(RECOVERY_DATA_START >= UINT32_C(0x10102000), "binding overlap");

bool recovery_target_allowed(uint64_t chip_id, uint32_t partition_start,
                             uint32_t partition_end) {
    return chip_id == RECOVERY_CHIP_ID &&
           partition_start == RECOVERY_DATA_START &&
           partition_end == RECOVERY_DATA_END;
}

int recovery_run(const recovery_ops *ops) {
    uint64_t chip_id = 0;
    recovery_partition firmware = {0}, data = {0}, binding = {0};
    if (!ops || !ops->chip_id || !ops->partition || !ops->erase ||
        ops->chip_id(&chip_id) != 0 ||
        ops->partition(0, &firmware) != 0 ||
        ops->partition(1, &data) != 0 ||
        ops->partition(2, &binding) != 0 ||
        firmware.start != RECOVERY_FIRMWARE_START || firmware.end != RECOVERY_FIRMWARE_END ||
        binding.start != RECOVERY_BINDING_START || binding.end != RECOVERY_BINDING_END ||
        !recovery_target_allowed(chip_id, data.start, data.end)) {
        return -1;
    }
    return ops->erase(RECOVERY_DATA_START, RECOVERY_DATA_END - RECOVERY_DATA_START);
}
