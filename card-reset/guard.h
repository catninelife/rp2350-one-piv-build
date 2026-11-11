#ifndef CARD_RECOVERY_RESET_GUARD_H
#define CARD_RECOVERY_RESET_GUARD_H

#include <stdbool.h>
#include <stdint.h>

#define RECOVERY_CHIP_ID UINT64_C(0x4d29f9224e4ed487)
#define RECOVERY_DATA_START UINT32_C(0x10102000)
#define RECOVERY_DATA_END UINT32_C(0x10400000)
#define RECOVERY_SECTOR_SIZE UINT32_C(0x1000)
#define RECOVERY_FIRMWARE_START UINT32_C(0x10000000)
#define RECOVERY_FIRMWARE_END UINT32_C(0x10100000)
#define RECOVERY_BINDING_START RECOVERY_FIRMWARE_END
#define RECOVERY_BINDING_END RECOVERY_DATA_START

typedef struct {
    uint32_t start;
    uint32_t end;
} recovery_partition;

typedef struct {
    int (*chip_id)(uint64_t *id);
    int (*partition)(unsigned index, recovery_partition *partition);
    int (*erase)(uint32_t start, uint32_t length);
} recovery_ops;

/* A decision only: this function does not read or modify a device. */
bool recovery_target_allowed(uint64_t chip_id, uint32_t partition_start,
                             uint32_t partition_end);

/* One guarded erase, or no erase at all. The caller handles returning to BOOTSEL. */
int recovery_run(const recovery_ops *ops);

#endif
