#!/usr/bin/env python3
"""Development: boots the game headless like play_first_level.sh but drives the control FIFO from
Python, so a test can read RAM dumps mid-run and react (e.g. find and move the player).
Usage: scripts/dev/drive.py SCRIPT.py  (SCRIPT defines run(ctl), called once the level is playable;
ctl.cmd(line), ctl.wait(s), ctl.dump(name) -> numpy float32 view of user RAM 0x08800000.)
Env: CA_PRESSES (menu presses), CA_CTL_DELAY (seconds before run), CA_SECONDS (total run time)."""
import os, subprocess, sys, time, runpy
import numpy as np
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
run_dir = os.path.join(root, 'build', 'run'); os.makedirs(run_dir, exist_ok=True); os.chdir(run_dir)
fifo = os.path.join(run_dir, 'drive.fifo')
if os.path.exists(fifo): os.remove(fifo)
os.mkfifo(fifo)
presses = os.environ.get('CA_PRESSES', '').split()
secs = int(os.environ.get('CA_SECONDS', '400'))
exe = os.environ.get('CA_PSPRUN', os.path.join(root, 'build', 'linux', 'ca_psprun'))
env = dict(os.environ, SDL_AUDIODRIVER='dummy')
log = open('drive.log', 'w')
p = subprocess.Popen(['xvfb-run', '-a', '-s', '-screen 0 1280x720x24', exe, '--iso', os.path.join(root, 'game', 'SyphonFilterDarkMirror.iso'),
                      '--gl', '--seconds', str(secs), '--control', fifo] + presses, stdout=log, stderr=subprocess.STDOUT, env=env)
f = open(fifo, 'w', buffering=1)
class Ctl:
    def cmd(self, line): f.write(line + '\n'); f.flush(); time.sleep(0.3)
    def wait(self, s): time.sleep(s)
    def f_write(self, line): f.write(line + '\n')  # no delay: several writes land in one frame
    def dump(self, name):
        path = os.path.join(run_dir, name)
        if os.path.exists(path): os.remove(path)
        self.cmd('memdump ' + name)
        for _ in range(100):
            if os.path.exists(path) and os.path.getsize(path) == 0x01800000: break
            time.sleep(0.1)
        time.sleep(0.3)
        return np.fromfile(path, dtype='<f4')
time.sleep(float(os.environ.get('CA_CTL_DELAY', '150')))
ns = runpy.run_path(sys.argv[1])
ns['run'](Ctl())
f.write('quit\n'); f.flush()
try: p.wait(timeout=60)
except subprocess.TimeoutExpired: p.kill()
