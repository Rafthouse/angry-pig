#pragma once
#if AP_DIAGNOSTICS
#include <windows.h>
#include <atomic>
#include <cstdio>
#include <string>
struct APMetrics {struct Entry{double us,rate;int frames,reset;};Entry entries[4096];std::atomic<unsigned long long>w{0},r{0},dropped{0};bool enabled=false;std::string file;double freq=1;
 void init(const char*dir,uintptr_t id){if(!dir)return;enabled=true;file=std::string(dir)+"/instance-"+std::to_string(id)+".csv";LARGE_INTEGER f;QueryPerformanceFrequency(&f);freq=(double)f.QuadPart;}
 void push(long long start,double rate,int frames,int reset){if(!enabled)return;LARGE_INTEGER t;QueryPerformanceCounter(&t);auto a=w.load(std::memory_order_relaxed);if(a-r.load(std::memory_order_acquire)>=4096){dropped++;return;}entries[a%4096]={1e6*(t.QuadPart-start)/freq,rate,frames,reset};w.store(a+1,std::memory_order_release);}
 void flush(){if(!enabled)return;auto a=r.load(),b=w.load(std::memory_order_acquire);if(a==b)return;FILE*f=fopen(file.c_str(),"a");if(!f)return;for(;a<b;a++){auto x=entries[a%4096];fprintf(f,"%.6f,%.0f,%d,%d\n",x.us,x.rate,x.frames,x.reset);}fclose(f);r.store(a,std::memory_order_release);}
};
struct APMeasure {APMetrics&m;double rate;int n;bool&reset;long long start=0;APMeasure(APMetrics&x,double sr,int frames,bool&re):m(x),rate(sr),n(frames),reset(re){if(m.enabled){LARGE_INTEGER t;QueryPerformanceCounter(&t);start=t.QuadPart;}}~APMeasure(){m.push(start,rate,n,reset);}};
#endif
