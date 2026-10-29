#pragma once
// Scoped floating-point policy abstraction.
//
// Windows x64: MXCSR (FTZ + DAZ, round-to-nearest).
// macOS x86_64: same MXCSR policy, available through <xmmintrin.h>.
// macOS arm64: FPCR (FZ, round-to-nearest); the architectural analogue of FTZ.
#if defined(_WIN32) && defined(_M_X64)
#include <immintrin.h>
struct APFloatingPoint { unsigned saved; explicit APFloatingPoint(bool flush=true):saved(_mm_getcsr()){unsigned x=(saved|0x1f80u)&~0x6000u;x=flush?(x|0x8040u):(x&~0x8040u);_mm_setcsr(x);} ~APFloatingPoint(){_mm_setcsr(saved);} APFloatingPoint(const APFloatingPoint&)=delete; };
#elif defined(__APPLE__) && (defined(__x86_64__) || defined(__i386__))
#include <xmmintrin.h>
struct APFloatingPoint { unsigned saved; explicit APFloatingPoint(bool flush=true):saved(_mm_getcsr()){unsigned x=(saved|0x1f80u)&~0x6000u;x=flush?(x|0x8040u):(x&~0x8040u);_mm_setcsr(x);} ~APFloatingPoint(){_mm_setcsr(saved);} APFloatingPoint(const APFloatingPoint&)=delete; };
#elif defined(__APPLE__) && defined(__aarch64__)
#include <cstdint>
namespace ap_fp_detail {
inline uint64_t read_fpcr() { uint64_t v; __asm__ __volatile__("mrs %0, fpcr" : "=r"(v)); return v; }
inline void write_fpcr(uint64_t v) { __asm__ __volatile__("msr fpcr, %0" :: "r"(v)); }
}
// FPCR: FZ = bit 24 (flush-to-zero), RMode = bits 22-23 (00 = nearest).
struct APFloatingPoint {
  uint64_t saved;
  explicit APFloatingPoint(bool flush = true) : saved(ap_fp_detail::read_fpcr()) {
    uint64_t x = saved & ~((1ULL << 22) | (1ULL << 23));
    x = flush ? (x | (1ULL << 24)) : (x & ~(1ULL << 24));
    ap_fp_detail::write_fpcr(x);
  }
  ~APFloatingPoint() { ap_fp_detail::write_fpcr(saved); }
  APFloatingPoint(const APFloatingPoint&) = delete;
};
#else
#error Supply and qualify a platform FP policy before enabling native DSP on this platform
#endif
