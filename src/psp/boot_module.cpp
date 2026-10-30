#include "psp/boot_module.h"

#include <cstdio>
#include <cstring>

#include "psp/iso.h"
#include "PrxDecrypter.h"

namespace psp {

ExeState classifyExe(const std::vector<uint8_t>& d) {
    if (d.empty()) return ExeState::Missing;
    if (d.size() >= 4 && std::memcmp(d.data(), "\x7f" "ELF", 4) == 0) return ExeState::Plain;
    if (d.size() >= 4 && std::memcmp(d.data(), "~PSP", 4) == 0) return ExeState::Encrypted;
    for (uint8_t b : d)
        if (b) return ExeState::Unknown;
    return ExeState::ZeroFilled;
}

const char* exeStateName(ExeState s) {
    switch (s) {
    case ExeState::Plain: return "plain ELF";
    case ExeState::ZeroFilled: return "zero-filled";
    case ExeState::Encrypted: return "encrypted (~PSP)";
    case ExeState::Missing: return "missing";
    default: return "unknown format";
    }
}

bool decryptPspContainer(const std::vector<uint8_t>& in, std::vector<uint8_t>& elf, std::string& why) {
    if (in.size() < sizeof(PSP_Header) || std::memcmp(in.data(), "~PSP", 4) != 0) {
        why = "not a ~PSP container";
        return false;
    }
    PSP_Header h;
    std::memcpy(&h, in.data(), sizeof h);
    if (h.psp_size > in.size() || h.psp_size < sizeof(PSP_Header)) {
        why = "truncated ~PSP container";
        return false;
    }
    // pspDecryptPRX works on a buffer of psp_size bytes and writes the result from offset 0.
    std::vector<uint8_t> buf(in.begin(), in.begin() + h.psp_size);
    std::vector<uint8_t> out(h.psp_size + 0x200);
    const int n = pspDecryptPRX(buf.data(), out.data(), h.psp_size);
    char tag[16];
    std::snprintf(tag, sizeof tag, "%08x", (unsigned)h.tag);
    if (n <= 0) {
        why = std::string("pspDecryptPRX failed (tag ") + tag + ")";
        return false;
    }
    out.resize((size_t)n);
    if (h.comp_attribute & 1) {
        // GZIP-compressed module (PPSSPP inflates it with zlib). Not seen on game discs so far.
        why = std::string("decrypted module is compressed (tag ") + tag + "), not supported yet";
        return false;
    }
    if (out.size() < 4 || std::memcmp(out.data(), "\x7f" "ELF", 4) != 0) {
        why = std::string("decryption produced no ELF (tag ") + tag + ")";
        return false;
    }
    elf.swap(out);
    why = std::string("decrypted, tag ") + tag;
    return true;
}

static std::vector<uint8_t> readFile(IsoReader& iso, const char* path) {
    IsoEntry e;
    std::vector<uint8_t> d;
    if (!iso.lookup(path, e) || e.dir) return d;
    d.resize(e.size);
    iso.read(e.lba, 0, d.data(), e.size);
    return d;
}

bool loadBootModule(IsoReader& iso, std::vector<uint8_t>& elf, std::string& how) {
    std::vector<uint8_t> boot = readFile(iso, "PSP_GAME/SYSDIR/BOOT.BIN");
    const ExeState bs = classifyExe(boot);
    if (bs == ExeState::Plain) {
        elf.swap(boot);
        how = "BOOT.BIN";
        return true;
    }
    std::string why;
    if (bs == ExeState::Encrypted && decryptPspContainer(boot, elf, why)) {
        how = "BOOT.BIN (" + why + ")";
        return true;
    }
    std::vector<uint8_t> eboot = readFile(iso, "PSP_GAME/SYSDIR/EBOOT.BIN");
    if (classifyExe(eboot) == ExeState::Plain) {
        elf.swap(eboot);
        how = "EBOOT.BIN (plain)";
        return true;
    }
    if (decryptPspContainer(eboot, elf, why)) {
        how = "EBOOT.BIN (" + why + ")";
        return true;
    }
    how = std::string("BOOT.BIN is ") + exeStateName(bs) + "; EBOOT.BIN: " + why;
    return false;
}

} // namespace psp
