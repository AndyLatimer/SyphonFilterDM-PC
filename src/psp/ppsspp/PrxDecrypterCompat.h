// Minimal stand-ins for the PPSSPP Common/ headers used by PrxDecrypter.cpp/.h (little-endian
// hosts only: x86-64 Windows and Linux).
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef u16 u16_le;
typedef u32 u32_le;
typedef s32 s32_le;
typedef u64 u64_le;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

namespace Log { enum { Loader }; }
#define SF_PRX_LOG(...) do { if (std::getenv("SF_PRX_VERBOSE")) { std::fprintf(stderr, "[prx] "); std::fprintf(stderr, __VA_ARGS__); std::fprintf(stderr, "\n"); } } while (0)
#define ERROR_LOG(cat, ...) SF_PRX_LOG(__VA_ARGS__)
#define WARN_LOG(cat, ...) SF_PRX_LOG(__VA_ARGS__)
#define INFO_LOG(cat, ...) SF_PRX_LOG(__VA_ARGS__)
#define DEBUG_LOG(cat, ...) SF_PRX_LOG(__VA_ARGS__)
#define VERBOSE_LOG(cat, ...) SF_PRX_LOG(__VA_ARGS__)
#define _dbg_assert_(x) ((void)0)
#define _dbg_assert_msg_(x, ...) ((void)0)
