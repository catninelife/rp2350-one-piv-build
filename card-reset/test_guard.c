#include "guard.h"
#include <assert.h>
#include <stdio.h>

static uint64_t id;
static recovery_partition partitions[3];
static int fail_stage, erase_count, erase_result;
static uint32_t erased_start, erased_length;
static int get_id(uint64_t *out) { if (fail_stage == 1) return -1; *out = id; return 0; }
static int get_partition(unsigned i, recovery_partition *out) {
    if (fail_stage == (int)i + 2) return -1;
    *out = partitions[i];
    return 0;
}
static int erase(uint32_t start, uint32_t length) {
    ++erase_count; erased_start = start; erased_length = length; return erase_result;
}
static const recovery_ops ops = {get_id, get_partition, erase};

static void reset_fixture(void) {
    id = RECOVERY_CHIP_ID;
    partitions[0] = (recovery_partition){RECOVERY_FIRMWARE_START, RECOVERY_FIRMWARE_END, RECOVERY_FIRMWARE_PERMISSIONS};
    partitions[1] = (recovery_partition){RECOVERY_DATA_START, RECOVERY_DATA_END, RECOVERY_DATA_PERMISSIONS};
    partitions[2] = (recovery_partition){RECOVERY_BINDING_START, RECOVERY_BINDING_END, RECOVERY_BINDING_PERMISSIONS};
    fail_stage = erase_count = erase_result = 0;
}

int main(void) {
    assert(recovery_target_allowed(RECOVERY_CHIP_ID, RECOVERY_DATA_START, RECOVERY_DATA_END));
    assert(!recovery_target_allowed(RECOVERY_CHIP_ID ^ 1u, RECOVERY_DATA_START, RECOVERY_DATA_END));
    assert(!recovery_target_allowed(RECOVERY_CHIP_ID, RECOVERY_DATA_START - RECOVERY_SECTOR_SIZE, RECOVERY_DATA_END));
    assert(!recovery_target_allowed(RECOVERY_CHIP_ID, RECOVERY_DATA_START + RECOVERY_SECTOR_SIZE, RECOVERY_DATA_END));
    assert(!recovery_target_allowed(RECOVERY_CHIP_ID, RECOVERY_DATA_START, RECOVERY_DATA_END + RECOVERY_SECTOR_SIZE));
    assert(!recovery_target_allowed(RECOVERY_CHIP_ID, RECOVERY_DATA_START, RECOVERY_DATA_END - RECOVERY_SECTOR_SIZE));
    reset_fixture();
    assert(recovery_run(&ops) == 0 && erase_count == 1);
    assert(erased_start == RECOVERY_DATA_START && erased_length == RECOVERY_DATA_END - RECOVERY_DATA_START);
    for (int i = 1; i <= 4; ++i) { reset_fixture(); fail_stage = i; assert(recovery_run(&ops) != 0 && erase_count == 0); }
    for (int i = 0; i < 3; ++i) {
        reset_fixture(); partitions[i].start -= RECOVERY_SECTOR_SIZE;
        assert(recovery_run(&ops) != 0 && erase_count == 0);
        reset_fixture(); partitions[i].end += RECOVERY_SECTOR_SIZE;
        assert(recovery_run(&ops) != 0 && erase_count == 0);
        reset_fixture(); partitions[i].permissions ^= UINT32_C(0x80000000);
        assert(recovery_run(&ops) != 0 && erase_count == 0);
    }
    reset_fixture(); id ^= 1u; assert(recovery_run(&ops) != 0 && erase_count == 0);
    reset_fixture(); erase_result = -7; assert(recovery_run(&ops) == -7 && erase_count == 1);
    assert(recovery_run(NULL) != 0);
    puts("target guard checks passed");
    return 0;
}
