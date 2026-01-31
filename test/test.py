#!/usr/bin/env python3

"""
Test the cpprc Pkzip implementation against the Python zlib one
"""

import traceback
import zlib
from argparse import ArgumentParser
from ctypes import CDLL, POINTER, c_char, c_size_t, c_uint32
from pathlib import Path
from random import Random

c_byte_p = POINTER(c_char)


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("lib", metavar="libtest.so", type=Path)
    parser.add_argument("--seed", default="1234", help="Random seed")
    parser.add_argument(
        "--count", default=1_000_000, type=int, help="Number of tests to run"
    )
    parser.add_argument("--min", default=1, type=int, help="Minimum data size")
    parser.add_argument("--max", default=256, type=int, help="Maximum data size")
    args = parser.parse_args()

    libtest = CDLL(args.lib)
    libtest.crc32Pkzip.argtypes = (c_byte_p, c_size_t)
    libtest.crc32Pkzip.restype = c_uint32

    random = Random(args.seed)

    print("Running")
    for i in range(args.count):
        try:
            print(f"{i}/{args.count}", end="\r")
            data = random.randbytes(random.randint(args.min, args.max))
            assert zlib.crc32(data) == libtest.crc32Pkzip(data, len(data))
        except Exception:
            print()
            traceback.print_exc()
    print(f"{args.count}/{args.count}")
    print("Done")


if __name__ == "__main__":
    main()
