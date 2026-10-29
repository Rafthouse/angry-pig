#define AP_PROBE_IMPL
#include "APProbe.h"
#include <atomic>
#include <new>
extern "C" {
#include "../dsp/probe.h"
}
static thread_local int rt=0;static std::atomic<unsigned long long> allocs{0},locks{0},io{0};
extern "C" __declspec(dllexport) void APSetRT(int v){rt=v;ap_processing=v;}
extern "C" __declspec(dllexport) unsigned long long APCounter(int i){return i==0?allocs.load()+ap_alloc_calls:i==1?locks.load()+ap_lock_calls:i==3?ap_grow_calls:io.load();}
extern "C" void* ap_malloc(size_t n){if(rt)allocs++;return malloc(n);}extern "C" void* ap_calloc(size_t n,size_t s){if(rt)allocs++;return calloc(n,s);}extern "C" void* ap_realloc(void* p,size_t n){if(rt)allocs++;return realloc(p,n);}extern "C" void ap_free(void* p){if(rt)allocs++;free(p);}extern "C" void ap_enter(LPCRITICAL_SECTION p){if(rt)locks++;EnterCriticalSection(p);}extern "C" FILE* ap_fopen(const char*a,const char*b){if(rt)io++;return fopen(a,b);}extern "C" size_t ap_fwrite(const void*a,size_t b,size_t c,FILE*f){if(rt)io++;return fwrite(a,b,c,f);}
void* operator new(size_t n){void* p=ap_malloc(n);if(!p)throw std::bad_alloc();return p;}void* operator new[](size_t n){return ::operator new(n);}void operator delete(void*p)noexcept{ap_free(p);}void operator delete[](void*p)noexcept{ap_free(p);}void operator delete(void*p,size_t)noexcept{ap_free(p);}void operator delete[](void*p,size_t)noexcept{ap_free(p);}
