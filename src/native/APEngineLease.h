#pragma once
#include <atomic>
// One bounded ownership attempt. 0=idle,1=process,2=quiescent control lifecycle.
struct APEngineLease {std::atomic<unsigned>& owner;bool held;APEngineLease(std::atomic<unsigned>&o,unsigned kind):owner(o){unsigned expected=0;held=owner.compare_exchange_strong(expected,kind,std::memory_order_acquire,std::memory_order_relaxed);}~APEngineLease(){if(held)owner.store(0,std::memory_order_release);}explicit operator bool()const{return held;}APEngineLease(const APEngineLease&)=delete;};
static_assert(std::atomic<unsigned>::is_always_lock_free,"engine lease must be lock free");
