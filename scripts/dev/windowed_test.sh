#!/usr/bin/env bash
# Development: runs the windowed PC build (SyphonFilterDarkMirror) on a virtual X display and
# drives it with real keyboard/mouse events (build/xsend, tools/xinput/xsend.c) from an xsend
# script, saving screenshots of the whole screen. Usage: scripts/dev/windowed_test.sh SCRIPT [SECONDS]
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
script=$(realpath "$1"); secs=${2:-120}
[ -x "$root/build/xsend" ] || cc -O2 "$root/tools/xinput/xsend.c" -o "$root/build/xsend" -lX11 -l:libXtst.so.6
out="$root/build/win_test"; mkdir -p "$out"; cd "$out"
disp=:77
Xvfb $disp -screen 0 1280x720x24 >/dev/null 2>&1 & xpid=$!
sleep 1
export DISPLAY=$disp SDL_AUDIODRIVER=dummy
CA_INPUT_DEBUG=1 timeout "$secs" "$root/build/linux/SyphonFilterDarkMirror" --iso "$root/game/SyphonFilterDarkMirror.iso" --windowed > game.log 2>&1 & gpid=$!
"$root/build/xsend" < "$script" || true
kill $gpid 2>/dev/null || true; wait $gpid 2>/dev/null || true
kill $xpid 2>/dev/null || true
grep -E "UNIMPL|error|Error" game.log | head || true
