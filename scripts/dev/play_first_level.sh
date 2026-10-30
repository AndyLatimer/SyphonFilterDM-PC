#!/usr/bin/env bash
# Development: boots the recompiled game headless (Xvfb), goes from the title screen to the
# first level of Story Mode and drives the player through ca_psprun's control FIFO, saving
# screenshots in build/run/play_*.png. Needs a profile already saved in build/run/memstick
# (create one first; see PROGRESS.md). Usage: scripts/dev/play_first_level.sh [extra seconds]
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
run="$root/build/run"; mkdir -p "$run"; cd "$run"
fifo="$run/ctl.fifo"; rm -f "$fifo"; mkfifo "$fifo"
extra=${1:-0}
P="--press CROSS@21 --press START@24 --press CROSS@28 --press DOWN@31 --press CROSS@32.5 --press CROSS@46 --press START@55 --press CROSS@57 --press CROSS@75"
SDL_AUDIODRIVER=dummy timeout $((240 + extra)) xvfb-run -a -s "-screen 0 1280x720x24" \
  "$root/build/linux/ca_psprun" --iso "$root/game/SyphonFilterDarkMirror.iso" --gl \
  --seconds $((170 + extra)) --control "$fifo" $P > play.log 2>&1 &
pid=$!
{
  sleep $((125 + extra)); echo "shot play_125.png"
  sleep 3; echo "shot play_128.png"; sleep 3; echo "shot play_131.png"
  echo "stick 128 0"; sleep 3; echo "shot play_134_fwd.png"; sleep 2; echo "shot play_136_fwd.png"
  echo "stick 0 128"; sleep 2; echo "shot play_138_left.png"
  echo "release"; echo "press R 0.3"; sleep 1; echo "shot play_140_r.png"
  echo "hold L"; sleep 1; echo "shot play_141_l.png"; echo "press R 0.3"; sleep 1; echo "shot play_142_lr.png"
  echo "release"; echo "press TRIANGLE 0.2"; sleep 2; echo "shot play_144_tri.png"
  echo "press START 0.2"; sleep 2; echo "shot play_146_pause.png"
  sleep 3
} > "$fifo"
wait $pid || true
grep -E "UNIMPL" play.log || true
ls play_*.png
