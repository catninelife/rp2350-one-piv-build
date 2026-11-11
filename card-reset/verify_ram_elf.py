"""Reject an ELF containing loadable bytes outside RP2350 SRAM."""
import struct
import sys
from pathlib import Path


def validate(path):
    data = Path(path).read_bytes()
    if data[:6] != b"\x7fELF\x01\x01":
        raise ValueError("expected a little-endian ELF32")
    entry = struct.unpack_from("<I", data, 24)[0]
    phoff = struct.unpack_from("<I", data, 28)[0]
    phentsize, phnum = struct.unpack_from("<HH", data, 42)
    if phentsize < 32 or phnum == 0 or phoff + phentsize * phnum > len(data):
        raise ValueError("invalid program header table")
    if not 0x20000000 <= (entry & ~1) < 0x20082000:
        raise ValueError("entry point is outside SRAM")
    loads = 0
    for index in range(phnum):
        p_type, offset, vaddr, paddr, filesz, memsz, _, _ = struct.unpack_from(
            "<IIIIIIII", data, phoff + index * phentsize
        )
        if p_type != 1:
            continue
        loads += 1
        if filesz > memsz or offset + filesz > len(data):
            raise ValueError("invalid load segment")
        for address in (vaddr, paddr):
            if not 0x20000000 <= address <= address + memsz <= 0x20082000:
                raise ValueError("load segment outside SRAM")
    if not loads:
        raise ValueError("no load segments")
    print(f"verified {loads} load segments and entry point within SRAM")


if __name__ == "__main__":
    validate(sys.argv[1])
