// Port hooks for Syphon Filter: Dark Mirror (UCES-00310), listed in
// tools/recomp/games/UCES00310/hooks.tsv. REVERSE_ENGINEERING.md section 14.

#include <algorithm>
#include <atomic>
#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "psp/host.h"
#include "psp/iso.h"
#include "psp/kernel.h"
#include "psp/memory.h"
#include "psp/recomp.h"

namespace {
float rdf(uint32_t a) { uint32_t v = psp::read32(a); float f; std::memcpy(&f, &v, 4); return f; }
void wrf(uint32_t a, float f) { uint32_t v; std::memcpy(&v, &f, 4); psp::write32(a, v); }
}

// The player's turn filter (0x08a1b208, called from 0x08a221d8 with the player's turn object),
// hooked at its end (0x08a1b2cc). Its input is the look buttons' direction (x = pitch, +1 up;
// z = yaw, -1 right); it ramps that up into a turn rate (+0x70 this frame, +0x30 the previous
// one), which the player and the camera turn with: about 2.2 rad/s per unit, measured from the
// camera matrix (REVERSE_ENGINEERING.md section 14). Direct aim replaces both with the rate that
// turns the view by exactly the mouse / right stick motion taken since the previous frame: no
// acceleration, no inertia, proportional to the stick.
constexpr float kRadPerSecPerUnit = 2.4f;

extern "C" void ca_hook_turn_rate(Ctx* c) {
    const uint32_t sp = c->r[29];
    if (psp::read32(sp + 64) != 0x08a221e0u) return; // only the player's turn
    const uint32_t obj = c->r[19];                      // s3
    static const bool trace = std::getenv("CA_HOOK_TRACE") != nullptr;
    static const float testYaw = std::getenv("CA_TEST_YAW_RATE") ? (float)std::atof(std::getenv("CA_TEST_YAW_RATE")) : 0.0f;
    static const float testPitch = std::getenv("CA_TEST_PITCH_RATE") ? (float)std::atof(std::getenv("CA_TEST_PITCH_RATE")) : 0.0f;
    if (testYaw != 0 || testPitch != 0) { // calibration runs
        if (testYaw != 0) { wrf(obj + 56, testYaw); wrf(obj + 120, testYaw); }
        if (testPitch != 0) { wrf(obj + 48, testPitch); wrf(obj + 112, testPitch); }
        return;
    }
    psp::host::g_directAim.store(true, std::memory_order_relaxed);
    psp::host::g_playerUpdates.fetch_add(1, std::memory_order_relaxed);

    static uint64_t lastUs = 0;
    const uint64_t now = psp::host::Aim::nowUs();
    const double dt = lastUs ? (now - lastUs) / 1e6 : 1.0 / 60;
    lastUs = now;
    double yaw = 0, pitch = 0;
    {
        std::lock_guard<std::mutex> lk(psp::host::g_aim.m);
        psp::host::g_aim.take(yaw, pitch);
    }
    // After a pause (menus, loading) the motion made meanwhile is dropped, not applied at once.
    if (dt > 0.1) yaw = pitch = 0;
    const float step = (float)std::clamp(dt, 1.0 / 240, 1.0 / 20) * kRadPerSecPerUnit;
    const float vy = std::clamp((float)(-yaw / step), -12.0f, 12.0f);
    const float vp = std::clamp((float)(-pitch / step), -12.0f, 12.0f);
    wrf(obj + 56, vy);
    wrf(obj + 120, vy);
    wrf(obj + 48, vp);
    wrf(obj + 112, vp);
    if (trace) std::fprintf(stderr, "[HOOK] turn obj=%08x dt=%.4f yaw=%.4f pitch=%.4f -> %.3f %.3f\n", obj, dt, yaw, pitch, vy, vp);
}

extern "C" void ca_hook_turn_input(Ctx*) {}
extern "C" void ca_hook_player_turn(Ctx*) {}
extern "C" void ca_hook_lua_pcall(Ctx*) {}

namespace psp::kernel {
void installTextPatches(IsoReader&) {}
}
