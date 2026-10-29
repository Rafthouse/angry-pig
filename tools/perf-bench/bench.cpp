// CPU profile and bit-exact regression of the Angry Pig DSP engine.
// Times the engine in AP_BASE_DIR (default src/native). To test a modified engine, copy it to another
// folder and build with /DAP_OPT_DIR=<folder>: every case is then compared sample by sample.
// x86-64 only (MXCSR flush-to-zero policy, as in the plug-in).
#ifndef AP_BASE_DIR
#define AP_BASE_DIR ../../src/native
#endif
#define AP_STR2(x) #x
#define AP_STR(x) AP_STR2(x)
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <immintrin.h>

namespace base {
#include AP_STR(AP_BASE_DIR/APDspEngine.h)
}
#ifdef AP_OPT_DIR
namespace opt {
#include AP_STR(AP_OPT_DIR/APDspEngine.h)
}
#else
namespace opt = base;
#endif

struct FP { unsigned saved; FP():saved(_mm_getcsr()){unsigned x=(saved|0x1f80u)&~0x6000u;x|=0x8040u;_mm_setcsr(x);} ~FP(){_mm_setcsr(saved);} };

using Clock=std::chrono::steady_clock;

// Test signals: 0 sine 999 Hz -12 dBFS; 1 program-like (bursts, noise, silence, levels); 2 digital silence.
static std::vector<float> makeSignal(int kind,double fs,int frames,int ch){
 std::vector<float> x(frames);
 uint32_t r=0x1234567u+ch;
 for(int n=0;n<frames;++n){
  const double t=n/fs;
  double v=0;
  if(kind==0)v=std::pow(10.,-12/20.)*std::sin(2*3.14159265358979323846*999.0*t+ch*0.3);
  else if(kind==1){
   r^=r<<13;r^=r>>17;r^=r<<5;
   const double noise=double(int32_t(r))/2147483648.0;
   const double seg=std::fmod(t,2.0);
   const double lvl=seg<0.5?0.05:seg<1.0?0.6:seg<1.3?0.0:1.4;
   v=lvl*(0.7*std::sin(2*3.14159265358979323846*(110.0+40*std::sin(t))*t)+0.3*noise);
  }
  x[n]=float(v);
 }
 return x;
}

struct Params{float p[4];};
// Parameter schedule: changes every ~0.37 s to exercise smoothing, color recompute and bypass.
static Params paramsAt(int scheduleKind,int n,double fs){
 if(scheduleKind==0)return {{50.f,20.f,0.f,0.f}};
 const int step=int(n/(0.37*fs));
 static const Params table[]={{{0.f,0.f,0.f,0.f}},{{100.f,80.f,0.f,0.f}},{{37.5f,55.f,12.f,0.f}},{{70.f,5.f,-30.f,0.f}},{{20.f,100.f,36.f,0.f}},{{60.f,40.f,-6.f,1.f}},{{90.f,10.f,6.f,0.f}}};
 return table[step%7];
}

template<class Engine>
static int render(Engine& e,double fs,int block,const std::vector<float>& l,const std::vector<float>& r,std::vector<float>& ol,std::vector<float>& orr,int sched,bool dynOff){
 FP fp;
 Params p0=paramsAt(sched,0,fs);
 e.dynamic.configure();
 if(dynOff)e.dynamic.amount=0;
 if(!e.prepare())return -100;
 e.dynamic.reset(fs);
 int rc=e.reset(float(fs),p0.p);
 if(rc)return rc;
 const int N=int(l.size());
 ol.assign(N,0);orr.assign(N,0);
 for(int at=0;at<N;at+=block){
  const int n=std::min(block,N-at);
  Params p=paramsAt(sched,at,fs);
  rc=e.process(l.data()+at,r.data()+at,ol.data()+at,orr.data()+at,n,p.p);
  if(rc)return rc;
 }
 return 0;
}

template<class Engine>
static double timeRender(double fs,int block,int sched,int kind,bool dynOff,double seconds,int reps){
 const int N=int(fs*seconds);
 auto l=makeSignal(kind,fs,N,0),r=makeSignal(kind,fs,N,1);
 std::vector<float> ol,orr;
 double best=1e30;
 for(int k=0;k<reps;++k){
  auto* e=new Engine();
  const auto t0=Clock::now();
  render(*e,fs,block,l,r,ol,orr,sched,dynOff);
  const double s=std::chrono::duration<double>(Clock::now()-t0).count();
  best=std::min(best,s);
  delete e;
 }
 return best*1e9/N; // ns per stereo frame
}

template<class Engine>
static double timeReset(double fs){
 FP fp;
 auto* e=new Engine();e->dynamic.configure();e->prepare();
 Params p=paramsAt(0,0,fs);
 const int K=2000;
 const auto t0=Clock::now();
 for(int k=0;k<K;++k){e->dynamic.reset(fs);e->reset(float(fs),p.p);}
 const double s=std::chrono::duration<double>(Clock::now()-t0).count();
 delete e;
 return s*1e6/K; // microseconds per reset
}

static volatile double sink;
static void micro(){
 FP fp;
 const int K=20000000;double acc=0;
 auto t0=Clock::now();for(int i=0;i<K;++i)acc+=std::pow(10.,(i&1023)*1e-4);double tPow=std::chrono::duration<double>(Clock::now()-t0).count()*1e9/K;
 t0=Clock::now();for(int i=0;i<K;++i)acc+=std::log10(1e-3+(i&1023)*1e-4);double tLog=std::chrono::duration<double>(Clock::now()-t0).count()*1e9/K;
 t0=Clock::now();for(int i=0;i<K;++i)acc+=std::sin((i&1023)*1e-3);double tSin=std::chrono::duration<double>(Clock::now()-t0).count()*1e9/K;
 t0=Clock::now();for(int i=0;i<K;++i)acc+=base::APDspEngine::saturate(0.5+(i&1023)*1.2e-3);double tSat=std::chrono::duration<double>(Clock::now()-t0).count()*1e9/K;
 sink=acc;
 std::printf("MICRO ns/call pow=%.2f log10=%.2f sin=%.2f saturate=%.2f\n",tPow,tLog,tSin,tSat);
}

static bool compareOne(double fs,int block,int sched,int kind,bool dynOff,double seconds,uint64_t& hash){
 const int N=int(fs*seconds);
 auto l=makeSignal(kind,fs,N,0),r=makeSignal(kind,fs,N,1);
 std::vector<float> bl,br,olv,orv;
 auto* b=new base::APDspEngine();auto* o=new opt::APDspEngine();
 const int rb=render(*b,fs,block,l,r,bl,br,sched,dynOff);
 const int ro=render(*o,fs,block,l,r,olv,orv,sched,dynOff);
 delete b;delete o;
 if(rb||ro){std::printf("  render error base=%d opt=%d\n",rb,ro);return false;}
 const bool same=std::memcmp(bl.data(),olv.data(),N*4)==0&&std::memcmp(br.data(),orv.data(),N*4)==0;
 uint64_t h=1469598103934665603ull;
 for(int n=0;n<N;++n){uint32_t a,c;std::memcpy(&a,&olv[n],4);std::memcpy(&c,&orv[n],4);h=(h^a)*1099511628211ull;h=(h^c)*1099511628211ull;}
 hash=h;
 if(!same){
  int first=-1;double maxd=0;
  for(int n=0;n<N;++n){double d=std::max(std::abs(double(bl[n])-olv[n]),std::abs(double(br[n])-orv[n]));if(d>0&&first<0)first=n;maxd=std::max(maxd,d);}
  std::printf("  DIFF first=%d maxabs=%.3e\n",first,maxd);
 }
 if(kind==2){ // digital silence must stay exact zero
  for(int n=0;n<N;++n)if(olv[n]!=0.f||orv[n]!=0.f){std::printf("  SILENCE NOT ZERO at %d\n",n);return false;}
 }
 return same;
}

int main(int argc,char** argv){
 const bool doTime=argc<2||std::strcmp(argv[1],"compare")!=0;
 const bool doCompare=argc<2||std::strcmp(argv[1],"time")!=0;
 int fails=0,cases=0;
 if(doCompare){
  const double rates[]={44100,48000,96000};
  const int blocks[]={1,7,64,512,4096};
  for(double fs:rates)for(int bl:blocks)for(int kind=0;kind<3;++kind)for(int sched=0;sched<2;++sched)for(int dynOff=0;dynOff<2;++dynOff){
   uint64_t h=0;const bool ok=compareOne(fs,bl,sched,kind,dynOff,3.0,h);
   ++cases;if(!ok){++fails;std::printf("CASE fs=%.0f block=%d kind=%d sched=%d dynOff=%d FAIL\n",fs,bl,kind,sched,dynOff);}
  }
  std::printf("COMPARE cases=%d bit_identical=%d failed=%d\n",cases,cases-fails,fails);
 }
 if(doTime){
  micro();
  const double rates[]={44100,48000,96000};
  const int blocks[]={1,64,512,4096};
  for(double fs:rates)for(int bl:blocks){
   const double tb=timeRender<base::APDspEngine>(fs,bl,1,1,false,20.0,5);
   const double to=timeRender<opt::APDspEngine>(fs,bl,1,1,false,20.0,5);
   const double tbOff=timeRender<base::APDspEngine>(fs,bl,1,1,true,20.0,3);
   std::printf("TIME fs=%.0f block=%4d base=%.1f ns/frame opt=%.1f ns/frame speedup=%.2fx base_dynOff=%.1f realtime_cpu_base=%.3f%% opt=%.3f%%\n",fs,bl,tb,to,tb/to,tbOff,tb*fs*1e-7,to*fs*1e-7);
  }
  for(double fs:rates)std::printf("RESET fs=%.0f base=%.2f us opt=%.2f us\n",fs,timeReset<base::APDspEngine>(fs),timeReset<opt::APDspEngine>(fs));
 }
 return fails?1:0;
}
