#!/usr/bin/env python3
"""Apply the reviewed 2MB board/partition profile to the pinned upstream tree."""
import argparse
import json
from pathlib import Path
import shutil


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise ValueError("pinned source differs from reviewed flash layout")
    return text.replace(old, new, 1)


def prepare(source, inputs):
    table = json.loads((inputs / "pt-zero-cm.json").read_text())
    assert table["partitions"][1]["start"] == "1032K"
    assert table["partitions"][1]["size"] == "1016K"
    shutil.copyfile(inputs / "pt-zero-cm.json", source / "pico-keys-sdk/config/rp2350/pt.json")
    path = source / "pico-keys-sdk/src/fs/low_flash.c"
    text = path.read_text()
    text = replace_once(text, "    FLASH_SIZE_BYTES = (1 << rxbuf[3]);", """    /* Zero-CM: never address an unknown or differently sized flash chip. */
    if (rxbuf[3] != 21) {
        reset_usb_boot(0, 0);
        return;
    }
    FLASH_SIZE_BYTES = (2u * 1024u * 1024u);""")
    text = replace_once(text, """    if (rc != 3) {
        data_start_addr = (FLASH_SIZE_BYTES >> 1);
        data_end_addr = FLASH_SIZE_BYTES;
    } else {""", """    if (rc != 3) {
        reset_usb_boot(0, 0);
        return;
    } else {""")
    anchor = "        data_start_addr = first_sector_number * FLASH_SECTOR_SIZE;"
    text = replace_once(text, anchor, """        /* Exact reviewed 2MB partition table, identical in A and B. */
        if (first_sector_number != 258 || last_sector_number != 511) {
            reset_usb_boot(0, 0);
            return;
        }
""" + anchor)
    path.write_text(text, newline="\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    args = parser.parse_args()
    prepare(args.source, Path(__file__).resolve().parents[1])
