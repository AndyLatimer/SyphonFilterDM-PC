#!/usr/bin/env python3
"""Recompiler driver helpers on synthetic images (no game needed): disc id from PARAM.SFO,
BOOT.BIN state detection, and the recompiler front end picking the main module.
Optional argument: path to sf_decrypt, to check it extracts the module from the same images."""
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "..", "tools", "recomp"))
from test_inspect_iso import make_iso, make_prx, make_sfo  # noqa: E402
import disc_id  # noqa: E402
import recompile  # noqa: E402


def image(boot, eboot, did="UCUS-98641"):
    return make_iso({
        "": {"UMD_DATA.BIN": (did + "|0123456789ABCDEF|0001|G\0").encode()},
        "PSP_GAME": {"PARAM.SFO": make_sfo([("DISC_ID", did.replace("-", "")), ("TITLE", "TEST")])},
        "PSP_GAME/SYSDIR": {"BOOT.BIN": boot, "EBOOT.BIN": eboot},
    })


def main():
    prx = make_prx()
    enc = b"~PSP" + b"\0" * 0x200
    cases = {"plain": (prx, enc), "zero-filled": (b"\0" * len(prx), prx), "encrypted": (enc, enc)}
    sf_decrypt = sys.argv[1] if len(sys.argv) > 1 else None
    failures = 0
    with tempfile.TemporaryDirectory() as t:
        for state, (boot, eboot) in cases.items():
            p = os.path.join(t, state + ".iso")
            open(p, "wb").write(image(boot, eboot))
            checks = {
                "disc id": recompile.disc_id(p) == "UCUS98641",
                "boot state": disc_id.boot_state(p) == state,
            }
            if sf_decrypt:
                out = os.path.join(t, state + ".elf")
                r = subprocess.run([sf_decrypt, p, out], capture_output=True, text=True)
                if state == "encrypted":  # garbage container: must fail cleanly with code 2
                    checks["sf_decrypt rejects"] = r.returncode == 2 and not os.path.exists(out)
                else:  # plain BOOT.BIN, or a plain EBOOT.BIN behind a zero-filled BOOT.BIN
                    checks["sf_decrypt extracts"] = r.returncode == 0 and open(out, "rb").read() == prx
            for k, ok in checks.items():
                print("%-12s %-20s %s" % (state, k, "ok" if ok else "FAIL"))
                failures += not ok
        # The recompiler refuses an unusable BOOT.BIN instead of translating garbage.
        p = os.path.join(t, "zero-filled.iso")
        r = subprocess.run([sys.executable, "-I", os.path.join(HERE, "..", "tools", "recomp", "recompile.py"),
                            "--iso", p, "--out", os.path.join(t, "gen")], capture_output=True, text=True)
        ok = r.returncode == 2 and "zero-filled" in r.stdout
        print("%-12s %-20s %s" % ("zero-filled", "recompile refuses", "ok" if ok else "FAIL"))
        failures += not ok
    print("FAILED" if failures else "all passed")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
