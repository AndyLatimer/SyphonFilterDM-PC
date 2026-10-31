#!/usr/bin/env python3
"""Disassembles part of the game's BOOT.BIN: disasm.py GAME.iso START END (hex addresses).
Prints address, word, mnemonic and operands as decoded by allegrex.py (development aid)."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from allegrex import decode  # noqa: E402
from iso9660 import read_iso_file  # noqa: E402
from pspelf import PspModule  # noqa: E402

REG = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
       "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]


def fmt(i):
    w = i.word
    op = w >> 26
    rs, rt, rd = (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
    imm = w & 0xFFFF
    simm = imm - 0x10000 if imm & 0x8000 else imm
    mn = i.mn
    if getattr(i, "is_branch", False):
        return "%-8s %s, %s -> %08x" % (mn, REG[rs], REG[rt], i.branch_target)
    if mn in ("j", "jal"):
        return "%-8s %08x" % (mn, i.target)
    if op in (0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B, 0x30, 0x38):
        return "%-8s %s, %d(%s)" % (mn, REG[rt], simm, REG[rs])
    if op in (0x31, 0x39):
        return "%-8s f%d, %d(%s)" % (mn, rt, simm, REG[rs])
    if op == 0x0F:
        return "%-8s %s, 0x%04x" % (mn, REG[rt], imm)
    if op in (0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E):
        return "%-8s %s, %s, %d" % (mn, REG[rt], REG[rs], simm if op in (8, 9, 10) else imm)
    if op == 0x11:  # FPU
        fs, fd, ft = (w >> 11) & 31, (w >> 6) & 31, (w >> 16) & 31
        return "%-8s f%d, f%d, f%d   (rt=%s)" % (mn, fd, fs, ft, REG[rt])
    if op == 0:
        return "%-8s %s, %s, %s  sa=%d" % (mn, REG[rd], REG[rs], REG[rt], (w >> 6) & 31)
    return mn


def main():
    mod = PspModule(read_iso_file(sys.argv[1], "PSP_GAME/SYSDIR/BOOT.BIN"))
    a0, a1 = int(sys.argv[2], 16), int(sys.argv[3], 16)
    for pc in range(a0, a1, 4):
        i = decode(mod.word(pc - mod.base), pc)
        print("%08x  %08x  %s" % (pc, i.word, fmt(i)))


if __name__ == "__main__":
    main()
