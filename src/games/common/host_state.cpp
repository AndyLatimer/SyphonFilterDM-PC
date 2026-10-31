// Front end <-> game state shared by every game (declared in psp/host.h). The game-specific
// hooks (src/games/<disc id>/game_hooks.cpp) read and write it; the front end sets it from
// the PC controls and settings. Moved here from the Coded Arms: Contagion hooks file so that
// a game without hooks yet (src/games/generic) still links.

#include "psp/host.h"

namespace psp::host {
Aim g_aim;
std::atomic<bool> g_ads{false};
AdsSettings g_adsSettings;
std::atomic<float> g_levelProjY{0};
std::atomic<uint32_t> g_weaponName{0};
std::atomic<uint32_t> g_playerUpdates{0};
LuaConsole g_luaConsole;
DevMode g_dev;
std::atomic<int> g_selectSlot{0};
GameOptions g_gameOptions;
std::atomic<int> g_targetFps{0};
std::atomic<bool> g_directAim{false};
std::atomic<int> g_vblankScale{1};
}
