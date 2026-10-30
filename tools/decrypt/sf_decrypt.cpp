// sf_decrypt GAME.iso OUT.BIN — writes the game's main module as a plain ELF: BOOT.BIN when it
// is in clear, otherwise EBOOT.BIN decrypted (src/psp/boot_module.cpp). The output is derived
// from your game: keep it in build/, never commit or share it.
// Exit codes: 0 written, 1 usage/IO error, 2 no usable module.
#include <cstdio>
#include <string>
#include <vector>

#include "psp/boot_module.h"
#include "psp/iso.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s GAME.iso OUT.BIN\n", argv[0]);
        return 1;
    }
    psp::IsoReader iso;
    if (!iso.open(argv[1])) {
        std::fprintf(stderr, "cannot open %s\n", argv[1]);
        return 1;
    }
    std::vector<uint8_t> elf;
    std::string how;
    if (!psp::loadBootModule(iso, elf, how)) {
        std::fprintf(stderr, "no usable main module: %s\n", how.c_str());
        return 2;
    }
    FILE* f = std::fopen(argv[2], "wb");
    if (!f || std::fwrite(elf.data(), 1, elf.size(), f) != elf.size()) {
        std::fprintf(stderr, "cannot write %s\n", argv[2]);
        return 1;
    }
    std::fclose(f);
    std::printf("%s: %zu bytes from %s\n", argv[2], elf.size(), how.c_str());
    return 0;
}
