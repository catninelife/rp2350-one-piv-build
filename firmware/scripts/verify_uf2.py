#!/usr/bin/env python3
"""Offline UF2 structure/address and A/B metadata verifier; never accesses hardware."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

ARM = 0xE48BFF59
ABSOLUTE = 0xE48BFF57


def inspect(path):
    raw = Path(path).read_bytes()
    if not raw or len(raw) % 512:
        raise ValueError("invalid UF2 length")
    blocks, seen, code, ancillary, indices, counts = [], set(), [], [], [], set()
    total = len(raw) // 512
    for pos in range(0, len(raw), 512):
        block = raw[pos:pos + 512]
        first, second, flags, address, size, index, count, family = struct.unpack_from("<8I", block)
        if (first, second, struct.unpack_from("<I", block, 508)[0]) != (0x0A324655, 0x9E5D5157, 0x0AB16F30):
            raise ValueError("invalid UF2 magic")
        if size != 256 or flags not in (0x2000, 0xA000) or count == 0 or index >= count:
            raise ValueError("invalid UF2 block header")
        if (family, address) in seen:
            raise ValueError("duplicate UF2 address")
        seen.add((family, address))
        if family == ARM:
            if flags != 0x2000 or address % 256 or not 0x10000000 <= address <= 0x10100000 - 256:
                raise ValueError("code outside 1MB firmware partition")
            code.append(address)
            indices.append(index)
            counts.add(count)
        elif family == ABSOLUTE and flags == 0xA000 and address == 0x10FFFF00 and index == 0 and count == 2:
            # Picotool's ignored absolute-family header, not a flash write.
            if pos != 0 or block[32:288] != b'\xef' * 256 or struct.unpack_from('<I', block, 288)[0] != 0x9957E304 or any(block[292:508]):
                raise ValueError("absolute header lacks reviewed RP2 ignore extension")
            ancillary.append(block[32:508])
        else:
            raise ValueError("unexpected UF2 family or flash target")
        blocks.append(block)
    if sorted(code) != list(range(0x10000000, 0x10000000 + len(code) * 256, 256)) or not code:
        raise ValueError("noncontiguous firmware")
    if counts != {len(code)} or indices != list(range(len(code))):
        raise ValueError("invalid ARM-family block sequence")
    if len(ancillary) != 1:
        raise ValueError("missing reviewed absolute ignore header")
    return {"sha256": hashlib.sha256(raw).hexdigest(), "blocks": total,
            "code_end": hex(max(code) + 256), "ancillary": [b.hex() for b in ancillary]}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("a", type=Path)
    parser.add_argument("b", type=Path, nargs="?")
    args = parser.parse_args()
    result = {"a": inspect(args.a)}
    if args.b:
        result["b"] = inspect(args.b)
        if result["a"]["ancillary"] != result["b"]["ancillary"]:
            raise SystemExit("A/B ancillary metadata mismatch")
    for entry in result.values():
        entry.pop("ancillary")
    print(json.dumps(result, indent=2))
