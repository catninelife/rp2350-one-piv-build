import contextlib
import io
import struct
import tempfile
import unittest
from pathlib import Path

from verify_ram_elf import validate


def image(entry=0x20000101, virtual=0x20000100, physical=0x20000100, size=16):
    data = bytearray(128)
    data[:6] = b"\x7fELF\x01\x01"
    struct.pack_into("<I", data, 24, entry)
    struct.pack_into("<I", data, 28, 52)
    struct.pack_into("<HH", data, 42, 32, 1)
    struct.pack_into("<IIIIIIII", data, 52, 1, 96, virtual, physical, size, size, 5, 4)
    return data


class RamElfValidation(unittest.TestCase):
    def run_validate(self, data):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "image.elf"
            path.write_bytes(data)
            with contextlib.redirect_stdout(io.StringIO()):
                validate(path)

    def test_accepts_sram_segments(self):
        self.run_validate(image())

    def test_rejects_flash_entry(self):
        with self.assertRaises(ValueError):
            self.run_validate(image(entry=0x10000101))

    def test_rejects_flash_virtual_or_load_address(self):
        for data in (image(virtual=0x10000000), image(physical=0x10000000)):
            with self.subTest(data=data), self.assertRaises(ValueError):
                self.run_validate(data)

    def test_rejects_segment_crossing_ram_end(self):
        with self.assertRaises(ValueError):
            self.run_validate(image(virtual=0x20081ff8, physical=0x20081ff8))

    def test_rejects_truncation(self):
        with self.assertRaises(ValueError):
            self.run_validate(image()[:60])


if __name__ == "__main__":
    unittest.main()
