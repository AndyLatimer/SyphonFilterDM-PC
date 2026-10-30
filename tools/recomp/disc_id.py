#!/usr/bin/env python3
"""Used by CMakeLists.txt.
  disc_id.py GAME.iso               DISC_ID without the dash (e.g. UCUS98641)
  disc_id.py --boot-state GAME.iso  plain | zero-filled | encrypted | missing | unknown (BOOT.BIN)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from iso9660 import read_iso_file  # noqa: E402
from recompile import disc_id  # noqa: E402


def boot_state(iso):
    try:
        d = read_iso_file(iso, "PSP_GAME/SYSDIR/BOOT.BIN")
    except Exception:
        return "missing"
    if d[:4] == b"\x7fELF":
        return "plain"
    if d[:4] == b"~PSP":
        return "encrypted"
    if not d.strip(b"\0"):
        return "zero-filled"
    return "unknown"


if __name__ == "__main__":
    if sys.argv[1] == "--boot-state":
        print(boot_state(sys.argv[2]))
    else:
        print(disc_id(sys.argv[1]))
