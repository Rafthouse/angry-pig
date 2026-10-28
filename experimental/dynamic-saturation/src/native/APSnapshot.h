#pragma once
#include <atomic>
#include <cstdint>
struct APValues {double v[4]={0,0,0,0};uint64_t version[4]{};uint64_t epoch=0;};
// One serialized writer per bank. A reader makes exactly one attempt; never spins.
struct APBank {std::atomic<uint64_t> seq{0},epoch{0},version[4];std::atomic<double> v[4];
 APBank(){APValues x;for(int i=0;i<4;i++){v[i]=x.v[i];version[i]=0;}}
 void write(const APValues& x){seq.fetch_add(1);for(int i=0;i<4;i++){v[i].store(x.v[i]);version[i].store(x.version[i]);}epoch.store(x.epoch);seq.fetch_add(1);}
 bool read(APValues& out)const{auto a=seq.load();if(a&1)return false;APValues x;for(int i=0;i<4;i++){x.v[i]=v[i].load();x.version[i]=version[i].load();}x.epoch=epoch.load();if(seq.load()!=a)return false;out=x;return true;}
};
static_assert(std::atomic<uint64_t>::is_always_lock_free,"audio mailbox must be lock free");
