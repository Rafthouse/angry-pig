#pragma once
// Scoped policy abstraction. Windows x64 qualified; other backends require qualification.
#if defined(_WIN32) && defined(_M_X64)
#include <immintrin.h>
struct APFloatingPoint { unsigned saved; explicit APFloatingPoint(bool flush=true):saved(_mm_getcsr()){unsigned x=(saved|0x1f80u)&~0x6000u;x=flush?(x|0x8040u):(x&~0x8040u);_mm_setcsr(x);} ~APFloatingPoint(){_mm_setcsr(saved);} APFloatingPoint(const APFloatingPoint&)=delete; };
#else
#error Supply and qualify a platform FP policy before enabling native DSP on this platform
#endif
