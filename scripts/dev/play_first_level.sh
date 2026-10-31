#!/usr/bin/env bash
# Development: boots the recompiled game headless (Xvfb), goes from the title screen to the
# first level of Story Mode and then feeds ca_psprun's control FIFO, saving screenshots in
# build/run/. Needs a profile already saved in build/run/memstick (see PROGRESS.md).
# Usage: scripts/dev/play_first_level.sh [COMMANDS_FILE] [EXTRA_ARGS...]
#   COMMANDS_FILE: lines "WAIT SECONDS" or a control command (press/hold/release/stick/shot ...),
#   sent once the player has control (~200 s after boot). Default: a short movement test.
#   EXTRA_ARGS go to ca_psprun (e.g. --wav out.wav).
# CA_PRESSES overrides the menu presses (training: scripts/dev/train_presses.txt), CA_CTL_DELAY the
# seconds before the commands start.
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
cmds=${1:+$(realpath "$1")}
shift || true
run="$root/build/run"; mkdir -p "$run"; cd "$run"
fifo="$run/ctl.fifo"; rm -f "$fifo"; mkfifo "$fifo"
default=$'shot play_0.png\nstick 0 128\nwait 2\nshot play_1_left.png\nrelease\nhold L\nwait 1\nshot play_2_l.png\nrelease'
script=$( [ -n "$cmds" ] && cat "$cmds" || echo "$default" )
total=$(awk 'tolower($1)=="wait"{s+=$2; next} NF{s+=0.3} END{print int(s)+220}' <<< "$script")
P=${CA_PRESSES:-"--press CROSS@21 --press START@24 --press CROSS@28 --press DOWN@31 --press CROSS@32.5 --press CROSS@46 --press START@55 --press CROSS@57 --press CROSS@75"}
SDL_AUDIODRIVER=dummy timeout $((total + 90)) xvfb-run -a -s "-screen 0 1280x720x24" \
  "${CA_PSPRUN:-$root/build/linux/ca_psprun}" --iso "$root/game/SyphonFilterDarkMirror.iso" --gl \
  --seconds "$total" --control "$fifo" $P "$@" > play.log 2>&1 &
pid=$!
{
  sleep "${CA_CTL_DELAY:-200}"
  while IFS= read -r line; do
    [ -z "$line" ] && continue
    if [ "$(tr '[:upper:]' '[:lower:]' <<< "${line%% *}")" = wait ]; then sleep "${line#* }"; else echo "$line"; sleep 0.3; fi
  done <<< "$script"
  sleep 2
} > "$fifo"
wait $pid || true
grep -E "UNIMPL" play.log || true
