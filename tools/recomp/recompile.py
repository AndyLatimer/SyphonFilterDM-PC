#!/usr/bin/env python3
"""Recompiles the main module of the user's own PSP game image into C sources.

Usage:
  recompile.py --iso GAME.iso --out build/generated
  recompile.py --iso GAME.iso --elf DECRYPTED_BOOT.BIN --out build/generated
  recompile.py --elf BOOT.BIN --game-id UCUS98641 --out build/generated

The game is identified by PARAM.SFO DISC_ID (or --game-id); games/<disc id>/ holds its hooks
and the extra modules recompiled with it. With --iso and --elf the code comes from --elf (a
BOOT.BIN decrypted from EBOOT.BIN, tools/decrypt) and the extra modules from the image.
The disc id is written to <out>/game_id.txt for the build.

The output is derived from the game's code: it is written to the build directory only
and must never be committed or distributed.
"""
import argparse
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import codegen  # noqa: E402
import functions  # noqa: E402
from iso9660 import read_iso_file  # noqa: E402
from pspelf import PspModule  # noqa: E402


def parse_sfo(d):
    """PARAM.SFO key -> value (strings and 32-bit integers)."""
    magic, ver, kofs, dofs, n = struct.unpack_from("<4sIIII", d, 0)
    if magic != b"\0PSF":
        raise ValueError("not a PARAM.SFO")
    out = {}
    for i in range(n):
        ko, fmt, ln, mx, do = struct.unpack_from("<HHIII", d, 20 + 16 * i)
        key = d[kofs + ko:d.index(b"\0", kofs + ko)].decode("latin-1")
        raw = d[dofs + do:dofs + do + ln]
        out[key] = struct.unpack_from("<I", raw)[0] if fmt == 0x0404 else raw.split(b"\0")[0].decode("utf-8", "replace")
    return out


def report_unknown(mod, funcs):
    """Functions holding undecodable words: usually data inside .text (see data.tsv)."""
    from allegrex import decode
    for f in sorted(funcs.values(), key=lambda f: f.start):
        n = sum(decode(mod.word(a - mod.base), a).mn == "unknown" for a in range(f.start, f.end, 4))
        if n:
            print("  %08x-%08x: %d unknown words%s" % (f.start, f.end, n, " (address taken)" if f.indirect else ""))


def load_data_ranges(game):
    """games/<disc id>/data.tsv: start, end (exclusive) of data blocks inside BOOT.BIN's .text
    that the function discovery must not take for code (their addresses are taken by the
    code, which looks like function pointers)."""
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "games", game, "data.tsv")
    out = []
    if os.path.exists(path):
        for line in open(path):
            line = line.split("#", 1)[0].split()
            if len(line) >= 2:
                out.append((int(line[0], 16), int(line[1], 16)))
    return out


def disc_id(iso):
    """DISC_ID of the image without the dash, e.g. 'UCUS98641'."""
    return str(parse_sfo(read_iso_file(iso, "PSP_GAME/PARAM.SFO")).get("DISC_ID", "generic")).replace("-", "")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--iso")
    ap.add_argument("--elf")
    ap.add_argument("--out", required=True)
    ap.add_argument("--trace", default="", help="comma separated function addresses to log (debugging)")
    ap.add_argument("--watch", action="store_true", help="debug: report stores to CA_WATCH_ADDR at run time")
    ap.add_argument("--profile", action="store_true", help="debug: count the calls of every function (profdump)")
    ap.add_argument("--game-id", default="", help="disc id without the dash (default: PARAM.SFO of --iso)")
    args = ap.parse_args()
    if not args.iso and not args.elf:
        ap.error("--iso or --elf is required")
    codegen.GAME_ID = args.game_id.replace("-", "") or (disc_id(args.iso) if args.iso else "generic")
    print("game id:", codegen.GAME_ID)
    if args.elf:
        data = open(args.elf, "rb").read()
    else:
        data = read_iso_file(args.iso, "PSP_GAME/SYSDIR/BOOT.BIN")
        if not data.strip(b"\0"):
            print("BOOT.BIN is zero-filled: decrypt EBOOT.BIN (tools/decrypt) and pass it with --elf")
            return 2
        if data[:4] == b"~PSP":
            print("BOOT.BIN is encrypted: decrypt it (tools/decrypt) and pass it with --elf")
            return 2
    os.makedirs(args.out, exist_ok=True)
    codegen.write_if_changed(os.path.join(args.out, "game_id.txt"), codegen.GAME_ID + "\n")
    mod = PspModule(data)
    mod.data_ranges = load_data_ranges(codegen.GAME_ID)
    for d0, d1 in mod.data_ranges:
        print("data in .text (not recompiled): %08x-%08x" % (d0, d1))
    funcs, starts = functions.discover(mod)
    modules = [(mod, funcs, starts, "")]
    # Further modules of the game recompiled with it (games/<disc id>/modules.tsv: path on the
    # disc, load address). They must come from the disc image.
    table = os.path.join(os.path.dirname(os.path.abspath(__file__)), "games", codegen.GAME_ID, "modules.tsv")
    if args.iso and os.path.exists(table):
        for line in open(table):
            line = line.split("#", 1)[0].split()
            if len(line) < 2:
                continue
            m = PspModule(read_iso_file(args.iso, line[0]), base=int(line[1], 16))
            mf, ms = functions.discover(m)
            modules.append((m, mf, ms, "disc0:/" + line[0]))
            print("module %s at %08x: functions: %d, imports: %d, exports: %d" % (line[0], m.base, len(mf), len(m.stubs), len(m.exports)))
    trace = list(funcs) if args.trace == "all" else [int(x, 16) for x in args.trace.split(",") if x]
    gen, files = codegen.generate_modules(modules, args.out, trace=trace, watch=args.watch, profile=args.profile)
    print("functions: %d, files: %d, imports: %d" % (len(funcs), len(files), len(mod.stubs)))
    if gen.unhandled:
        print("UNHANDLED mnemonics:", gen.unhandled)
        report_unknown(mod, funcs)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
