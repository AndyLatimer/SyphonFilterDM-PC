#pragma once
// The game's main module as the recompiler and the runtime both need it: a plain ELF/PRX.
// Most discs carry it in clear as PSP_GAME/SYSDIR/BOOT.BIN; on others BOOT.BIN is zero-filled
// or encrypted and only the encrypted EBOOT.BIN (~PSP) is usable, which is then decrypted with
// PPSSPP's pspDecryptPRX (src/psp/ppsspp/PrxDecrypter.cpp) and libkirk.
#include <cstdint>
#include <string>
#include <vector>

namespace psp {
class IsoReader;

enum class ExeState { Plain, ZeroFilled, Encrypted, Missing, Unknown };
ExeState classifyExe(const std::vector<uint8_t>& data);
const char* exeStateName(ExeState s);

// Decrypts a ~PSP container into a plain ELF; false (with a reason) if it cannot.
bool decryptPspContainer(const std::vector<uint8_t>& in, std::vector<uint8_t>& elf, std::string& why);

// The main module from the disc: BOOT.BIN when it is plain, otherwise EBOOT.BIN decrypted.
// `how` says which ("BOOT.BIN", "EBOOT.BIN (decrypted, tag XXXXXXXX)") or why it failed.
bool loadBootModule(IsoReader& iso, std::vector<uint8_t>& elf, std::string& how);
}
