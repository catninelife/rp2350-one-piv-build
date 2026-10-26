import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("verify_uf2", Path(__file__).parents[1] / "scripts/verify_uf2.py")
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)


def block(address=0x10000000, family=verify.ARM, index=0, total=1, flags=0x2000):
    raw = bytearray(512)
    struct.pack_into("<8I", raw, 0, 0x0A324655, 0x9E5D5157, flags, address, 256, index, total, family)
    struct.pack_into("<I", raw, 508, 0x0AB16F30)
    if family == verify.ABSOLUTE and flags == 0xA000:
        raw[32:288] = b'\xef' * 256
        struct.pack_into('<I', raw, 288, 0x9957E304)
    return raw


def ignored():
    return block(address=0x10FFFF00, family=verify.ABSOLUTE, total=2, flags=0xA000)


class UF2Tests(unittest.TestCase):
    def check(self, data):
        with tempfile.TemporaryDirectory() as directory:
            p = Path(directory) / "synthetic.uf2"
            p.write_bytes(data)
            return verify.inspect(p)

    def test_valid_minimal_code(self):
        self.assertEqual(self.check(ignored() + block())["code_end"], "0x10000100")

    def test_reject_data_region_write(self):
        with self.assertRaises(ValueError):
            self.check(block(address=0x10102000))

    def test_reject_foreign_family_and_absolute_write(self):
        for family, flags in ((0xE48BFF56, 0x2000), (verify.ABSOLUTE, 0x2000)):
            with self.assertRaises(ValueError):
                self.check(block(family=family, flags=flags))

    def test_reject_duplicates_gap_count_and_truncation(self):
        for raw in (block(total=2) + block(index=1, total=2), block(address=0x10000100), block(total=2), block()[:-1]):
            with self.assertRaises(ValueError):
                self.check(raw)

    def test_only_ignored_absolute_header_allowed(self):
        # Picotool numbers/counts each family independently, not the whole file.
        raw = block(address=0x10FFFF00, family=verify.ABSOLUTE, total=2, flags=0xA000) + block()
        self.assertEqual(self.check(raw)["blocks"], 2)

    def test_family_sequence_rejects_missing_and_reordered_arm_blocks(self):
        for raw in (block(index=1, total=2), block(total=3) + block(address=0x10000100, index=1, total=3)):
            with self.assertRaises(ValueError):
                self.check(raw)

    def test_reject_absolute_header_without_ignore_extension(self):
        for offset in (32, 288, 292):
            raw = ignored()
            raw[offset] ^= 1
            with self.assertRaises(ValueError):
                self.check(raw + block())

    def test_reject_missing_ignore_header(self):
        with self.assertRaises(ValueError):
            self.check(block())
