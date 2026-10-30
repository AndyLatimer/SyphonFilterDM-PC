// Hooks for a game that has none yet: the recompiled code runs unmodified with the plain
// PSP controls (the front end maps the PC keys and the pad to PSP buttons and the analog
// stick). Used until src/games/<disc id>/game_hooks.cpp exists for the user's disc; the
// build picks that file automatically (CMakeLists.txt, SF_GAME_ID).

#include "psp/iso.h"
#include "psp/kernel.h"
#include "psp/recomp.h"

// Declared in psp/recomp.h for the Contagion hooks (docs/cac); never called without a
// games/<disc id>/hooks.tsv that names them.
extern "C" void ca_hook_player_turn(Ctx*) {}
extern "C" void ca_hook_lua_pcall(Ctx*) {}

namespace psp::kernel {
void installTextPatches(IsoReader&) {}
}
