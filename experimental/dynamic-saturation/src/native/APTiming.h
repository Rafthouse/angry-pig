#pragma once
#include "../qualification/wp_timing.h"
#if AP_TIMING
#include <windows.h>
#include <atomic>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
struct WPRecord {uint64_t id=0,seq=0,setup=0,epoch=0,start=0,end=0,s[10]{};double rate=0;uint32_t tid=0,kind=0,frames=0,queues=0,calls=0,reset=0,fault=0;};
struct WPFrame {WPRecord r;uint64_t begin[10]{};};
extern thread_local WPFrame* wp_frame;
inline uint64_t wp_now(){LARGE_INTEGER t;QueryPerformanceCounter(&t);return t.QuadPart;}
struct WPTiming {static constexpr unsigned Capacity=8192;std::array<WPRecord,Capacity> records{};std::atomic<uint64_t> used{0},dropped{0},sequence{0},setup{0};uint64_t id=0,freq=1;bool enabled=false;
 void init(){static std::atomic<uint64_t> ids{0};id=++ids;LARGE_INTEGER f;QueryPerformanceFrequency(&f);freq=f.QuadPart;enabled=std::getenv("AP_WP1_TIMING")!=nullptr;}
 void dump(const char*path){FILE*f=fopen(path,"w");if(!f)return;auto n=used.load();fprintf(f,"{\"meta\":true,\"id\":%llu,\"frequency\":%llu,\"capacity\":%u,\"reserved\":%llu,\"dropped\":%llu}\n",id,freq,Capacity,n,dropped.load());for(uint64_t i=0;i<n&&i<Capacity;i++){auto&r=records[i];fprintf(f,"{\"id\":%llu,\"seq\":%llu,\"setup\":%llu,\"epoch\":%llu,\"start\":%llu,\"end\":%llu,\"tid\":%u,\"kind\":%u,\"frames\":%u,\"rate\":%.0f,\"queues\":%u,\"calls\":%u,\"reset\":%u,\"fault\":%u,\"stages\":[",r.id,r.seq,r.setup,r.epoch,r.start,r.end,r.tid,r.kind,r.frames,r.rate,r.queues,r.calls,r.reset,r.fault);for(int j=0;j<10;j++)fprintf(f,"%s%llu",j?",":"",r.s[j]);fprintf(f,"]}\n");}fclose(f);}
};
struct WPScope {WPTiming& buffer;WPFrame frame;WPFrame* previous=nullptr;bool active=false;
 WPScope(WPTiming&b,unsigned kind,double rate,int frames,uint64_t epoch):buffer(b){if(!b.enabled)return;if(b.used.load(std::memory_order_relaxed)>=WPTiming::Capacity){b.dropped++;return;}active=true;previous=wp_frame;frame.r.id=b.id;frame.r.seq=b.sequence.fetch_add(1);frame.r.setup=b.setup.load();frame.r.epoch=epoch;frame.r.kind=kind;frame.r.rate=rate;frame.r.frames=frames;frame.r.tid=GetCurrentThreadId();frame.r.start=wp_now();wp_frame=&frame;}
 ~WPScope(){if(!active)return;frame.r.end=wp_now();for(int j=0;j<10;j++)if(frame.begin[j])frame.r.s[j]+=frame.r.end-frame.begin[j];wp_frame=previous;auto i=buffer.used.fetch_add(1);if(i<WPTiming::Capacity)buffer.records[i]=frame.r;else buffer.dropped++;}
};
#define WP_SCOPE(b,k,r,n,e) WPScope wpScope(b,k,r,n,e)
#define WP_STAGE(s) wp_begin(s)
#define WP_END(s) wp_end(s)
#else
#define WP_SCOPE(b,k,r,n,e) ((void)0)
#define WP_STAGE(s) ((void)0)
#define WP_END(s) ((void)0)
#endif
