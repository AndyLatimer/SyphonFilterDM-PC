#include "psp/ge.h"

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <map>
#include <thread>

#include "psp/kernel.h"
#include "psp/memory.h"

namespace psp::ge {

namespace {

enum : uint32_t {
    CMD_NOP = 0x00, CMD_JUMP = 0x08, CMD_BJUMP = 0x09, CMD_CALL = 0x0A, CMD_RET = 0x0B,
    CMD_END = 0x0C, CMD_SIGNAL = 0x0E, CMD_FINISH = 0x0F, CMD_BASE = 0x10, CMD_OFFSET = 0x13,
    CMD_ORIGIN = 0x14,
};

struct Callback {
    uint32_t signalFunc = 0, signalArg = 0, finishFunc = 0, finishArg = 0;
};

std::recursive_mutex g_mutex;
Renderer* g_renderer = nullptr;
std::map<int, List> g_lists;
std::map<int, Callback> g_callbacks;
int g_nextList = 1, g_nextCb = 1;
uint32_t g_regs[256];

uint32_t target(const List& l, uint32_t data) { return ((l.base | data) + l.offset) & 0x0FFFFFFC; }

// Runs a list until it ends or reaches its stall address.
void run(List& l) {
    while (!l.done) {
        if (l.stall && (l.pc & 0x0FFFFFFF) == (l.stall & 0x0FFFFFFF)) return;
        uint32_t w = read32(l.pc);
        uint32_t cmd = w >> 24, data = w & 0xFFFFFF;
        uint32_t prev = g_regs[cmd];
        g_regs[cmd] = data;
        uint32_t pc = l.pc;
        l.pc += 4;
        switch (cmd) {
        case CMD_BASE: l.base = (data << 8) & 0x0F000000; break; // bits 16-19 -> address bits 24-27
        case CMD_OFFSET: l.offset = data << 8; break;
        case CMD_ORIGIN: l.offset = pc; break;
        case CMD_JUMP: l.pc = target(l, data); break;
        case CMD_BJUMP: break; // bounding-box test: treated as visible (no jump)
        case CMD_CALL:
            if (l.callDepth < 32) l.callStack[l.callDepth++] = l.pc;
            l.pc = target(l, data);
            break;
        case CMD_RET:
            if (l.callDepth > 0) l.pc = l.callStack[--l.callDepth];
            break;
        case CMD_SIGNAL: {
            l.signalArg = data & 0xFFFF;
            auto it = g_callbacks.find(l.cbid);
            if (it != g_callbacks.end() && it->second.signalFunc)
                kernel::queueGuestCall(it->second.signalFunc, l.signalArg, it->second.signalArg, 0);
            break;
        }
        case CMD_FINISH:
            l.finishing = true;
            l.signalArg = data & 0xFFFF;
            break;
        case CMD_END:
            if (l.finishing) {
                l.done = true;
                if (g_renderer) g_renderer->listFinished();
                auto it = g_callbacks.find(l.cbid);
                if (it != g_callbacks.end() && it->second.finishFunc)
                    kernel::queueGuestCall(it->second.finishFunc, l.signalArg, it->second.finishArg, 0);
                kernel::notifyStateChanged();
            }
            break;
        default:
            if (g_renderer) g_renderer->command(pc, cmd, data, prev, ((l.base | data) + l.offset) & 0x0FFFFFFF);
            break;
        }
    }
}

} // namespace

// ---- asynchronous processing ----------------------------------------------------------------
// On the PSP the GE walks display lists in parallel with the CPU. Running them on the calling
// guest thread (CA_GE_SYNC=1) adds all the translation work to the game's frame; by default a
// GE thread processes them, in submission order, and the sync calls wait for it.
std::mutex g_qm;
std::condition_variable g_qcv, g_idleCv2;
std::deque<int> g_queue;      // list ids with work (new list, or a moved stall address)
std::atomic<int> g_busy{0};   // the GE thread is running a list
bool g_threadStarted = false;
bool asyncMode() { static const bool sync = std::getenv("CA_GE_SYNC") != nullptr; return !sync; }

void geThread() {
    for (;;) {
        int id;
        {
            std::unique_lock<std::mutex> lk(g_qm);
            g_qcv.wait(lk, [] { return !g_queue.empty(); });
            id = g_queue.front();
            g_queue.pop_front();
            g_busy = 1;
        }
        {
            std::lock_guard<std::recursive_mutex> lk(g_mutex);
            auto it = g_lists.find(id);
            if (it != g_lists.end() && !it->second.done) run(it->second);
        }
        {
            std::lock_guard<std::mutex> lk(g_qm);
            g_busy = 0;
            g_idleCv2.notify_all();
        }
        kernel::notifyStateChanged();
    }
}

void submit(int id) {
    std::lock_guard<std::mutex> lk(g_qm);
    if (!g_threadStarted) { g_threadStarted = true; std::thread(geThread).detach(); }
    g_queue.push_back(id);
    g_qcv.notify_one();
}


void waitIdle() {
    if (!asyncMode()) return;
    std::unique_lock<std::mutex> lk(g_qm);
    g_idleCv2.wait(lk, [] { return g_queue.empty() && !g_busy; });
}

void setRenderer(Renderer* r) {
    std::lock_guard<std::recursive_mutex> lk(g_mutex);
    g_renderer = r;
}

uint32_t reg(int cmd) { return g_regs[cmd & 0xFF]; }

int enqueue(uint32_t list, uint32_t stall, int cbid, bool head) {
    (void)head;
    int id;
    {
        std::lock_guard<std::recursive_mutex> lk(g_mutex);
        List l;
        l.id = id = g_nextList++;
        l.start = l.pc = list & 0x0FFFFFFF;
        l.stall = stall & 0x0FFFFFFF;
        l.cbid = cbid;
        auto& ref = g_lists[l.id] = l;
        if (!asyncMode()) { run(ref); return id; }
    }
    submit(id);
    return id;
}

int updateStall(int id, uint32_t stall) {
    {
        std::lock_guard<std::recursive_mutex> lk(g_mutex);
        auto it = g_lists.find(id);
        if (it == g_lists.end()) return -1;
        it->second.stall = stall & 0x0FFFFFFF;
        if (!asyncMode()) { run(it->second); return 0; }
    }
    submit(id);
    return 0;
}

bool listDone(int id) {
    std::lock_guard<std::recursive_mutex> lk(g_mutex);
    auto it = g_lists.find(id);
    return it == g_lists.end() || it->second.done;
}

bool allDone() {
    if (asyncMode()) {
        std::lock_guard<std::mutex> lk(g_qm);
        if (!g_queue.empty() || g_busy) return false;
    }
    std::lock_guard<std::recursive_mutex> lk(g_mutex);
    for (auto it = g_lists.begin(); it != g_lists.end();) {
        if (!it->second.done) return false;
        it = g_lists.erase(it);
    }
    return true;
}

int setCallback(uint32_t signalFunc, uint32_t signalArg, uint32_t finishFunc, uint32_t finishArg) {
    std::lock_guard<std::recursive_mutex> lk(g_mutex);
    int id = g_nextCb++;
    g_callbacks[id] = {signalFunc, signalArg, finishFunc, finishArg};
    return id;
}

void unsetCallback(int cbid) {
    std::lock_guard<std::recursive_mutex> lk(g_mutex);
    g_callbacks.erase(cbid);
}

} // namespace psp::ge
