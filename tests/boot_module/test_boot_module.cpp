// classifyExe / decryptPspContainer on synthetic data (no game image): formats are told apart
// and bad containers are rejected without crashing. Real decryption is only checked with the
// user's disc (TESTING.md).
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "psp/boot_module.h"

static int g_fail = 0;
static void check(const char* what, bool ok) {
    std::printf("%-40s %s\n", what, ok ? "ok" : "FAIL");
    g_fail += !ok;
}

int main() {
    using psp::ExeState;
    std::vector<uint8_t> elf = {0x7f, 'E', 'L', 'F', 1, 1, 1, 0};
    std::vector<uint8_t> zero(4096, 0);
    std::vector<uint8_t> psp(0x400, 0);
    std::memcpy(psp.data(), "~PSP", 4);
    std::vector<uint8_t> junk = {1, 2, 3, 4};
    check("plain ELF", psp::classifyExe(elf) == ExeState::Plain);
    check("zero-filled", psp::classifyExe(zero) == ExeState::ZeroFilled);
    check("encrypted", psp::classifyExe(psp) == ExeState::Encrypted);
    check("missing", psp::classifyExe({}) == ExeState::Missing);
    check("unknown", psp::classifyExe(junk) == ExeState::Unknown);

    std::vector<uint8_t> out;
    std::string why;
    check("rejects non-container", !psp::decryptPspContainer(elf, out, why));
    check("rejects truncated header", !psp::decryptPspContainer({'~', 'P', 'S', 'P'}, out, why));
    // psp_size (0x2C) larger than the data
    std::vector<uint8_t> big = psp;
    uint32_t sz = 0x100000;
    std::memcpy(&big[0x2C], &sz, 4);
    check("rejects psp_size past the end", !psp::decryptPspContainer(big, out, why));
    // well-formed size, but no valid tag/keys: pspDecryptPRX must fail cleanly
    sz = (uint32_t)psp.size();
    std::memcpy(&psp[0x2C], &sz, 4);
    check("rejects unknown tag", !psp::decryptPspContainer(psp, out, why) && out.empty());
    std::printf("%s\n", g_fail ? "FAILED" : "all passed");
    return g_fail ? 1 : 0;
}
