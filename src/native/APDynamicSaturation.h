#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

// Dynamic Saturation candidate (isolated build, own VST3 class ID).
// Configuration is read at construction, never on the audio thread.
// No public parameter or serialized state is added.
//
// A  slow additive operating-point bias (unchanged from the Dynamic EXP).
// B  additive, band-limited (32 Hz, 2nd-order Butterworth) seeded modulation of the saturator
//    input, scaled by the input envelope. Replaces the Dynamic EXP's 0.19 Hz multiplicative drive,
//    which moved only sub-hertz sidebands and therefore produced no halo in the 1 Hz shells.
// C  broadband background (unchanged from the Dynamic EXP). Off by default in this candidate
//    (AP_DYNAMIC_COMPONENTS=ABC enables it); its low-frequency part duplicates B.
//
// Default amount is 1. AP_DYNAMIC_AMOUNT=0 disables every term: the output is the RC1.1 path, bit for bit.
struct APDynamicSaturation {
 static constexpr double kPi=3.14159265358979323846;
 double amount=1.0, rate=48000, phase=0, gate=0;
 double gatePole=0, lowPole=0, widePole=0, dcPole=0, outputPole=0, envPole=0;
 double low[2]{}, low2[2]{}, wide[2]{}, dc[2]{}, dcIn[2]{};
 double outputIn[2]{}, outputHp[2]{};
 double haloEnv[2]{}, haloZ1[2]{}, haloZ2[2]{};
 double haloB0=0, haloB1=0, haloB2=0, haloA1=0, haloA2=0, haloNorm=0;
 uint32_t rng[2]{0x13579bdfu,0x2468ace1u};
 uint32_t haloRng[2]{0x3c6ef372u,0x82d1a5e7u};
 uint32_t haloSeed=0x2545f491u;
 double haloScale=1.0;      // test-only multiplier (AP_DYNAMIC_HALO_SCALE), 1 in the shipped candidate
 double haloKappa=2.0e-4;   // bias per unit envelope at amount 1; calibrated once on the -12 dBFS sine
 bool bias=true, halo=true, residual=false;   // C is off in this candidate: its low-frequency part duplicates B and raises the floor
 bool lowPath=true;                             // test-only (AP_DYNAMIC_C_LOWPATH=0): C without its 70 Hz low-frequency path

 void configure() noexcept {
  if(const char* s=std::getenv("AP_DYNAMIC_AMOUNT")) {
   char* end=nullptr; const double v=std::strtod(s,&end);
   if(end!=s && *end=='\0' && std::isfinite(v))amount=std::clamp(v,0.0,1.0);
  }
  if(const char* s=std::getenv("AP_DYNAMIC_COMPONENTS")) {
   bias=std::strchr(s,'A')!=nullptr;
   halo=std::strchr(s,'B')!=nullptr;
   residual=std::strchr(s,'C')!=nullptr;
  }
  if(const char* s=std::getenv("AP_DYNAMIC_C_LOWPATH")) lowPath=s[0]!='0';
  if(const char* s=std::getenv("AP_DYNAMIC_HALO_SCALE")) {
   char* end=nullptr; const double v=std::strtod(s,&end);
   if(end!=s && *end=='\0' && std::isfinite(v) && v>=0.0)haloScale=v;
  }
  if(const char* s=std::getenv("AP_DYNAMIC_SEED")) {
   char* end=nullptr; const unsigned long v=std::strtoul(s,&end,10);
   if(end!=s && *end=='\0' && v!=0ul)haloSeed=uint32_t(v);
  }
 }
 bool enabled() const noexcept {return amount>0 && (bias||halo||residual);}
 void reset(double fs) noexcept {
  rate=fs;phase=gate=0;
  gatePole=std::exp(-1.0/(rate*.030));
  envPole=std::exp(-1.0/(rate*.001));
  lowPole=std::exp(-2.0*kPi*70.0/rate);
  widePole=std::exp(-2.0*kPi*2500.0/rate);
  dcPole=std::exp(-2.0*kPi*15.0/rate);
  outputPole=std::exp(-2.0*kPi*3.0/rate);
  for(int c=0;c<2;++c){
   low[c]=low2[c]=wide[c]=dc[c]=dcIn[c]=outputIn[c]=outputHp[c]=0;
   haloEnv[c]=haloZ1[c]=haloZ2[c]=0;
  }
  rng[0]=0x13579bdfu;rng[1]=0x2468ace1u;
  haloRng[0]=haloSeed;
  haloRng[1]=haloSeed^0x9e3779b9u;
  if(haloRng[1]==0u)haloRng[1]=0x82d1a5e7u;
  // 2nd-order Butterworth low-pass at 32 Hz (bilinear transform).
  const double K=std::tan(kPi*32.0/rate);
  const double norm=1.0/(1.0+std::sqrt(2.0)*K+K*K);
  haloB0=K*K*norm;haloB1=2.0*haloB0;haloB2=haloB0;
  haloA1=2.0*(K*K-1.0)*norm;haloA2=(1.0-std::sqrt(2.0)*K+K*K)*norm;
  // Scale the filtered output to unit variance for a uniform white input (variance 1/3).
  double z1=0,z2=0,energy=0;
  for(int n=0;n<8192;++n){
   const double x=n==0?1.0:0.0;
   const double y=haloB0*x+z1;
   z1=haloB1*x-haloA1*y+z2;z2=haloB2*x-haloA2*y;
   energy+=y*y;
  }
  haloNorm=std::sqrt(3.0/energy);
 }
 void advance(double peak) noexcept {
  phase+=2.0*kPi*.071/rate;
  if(phase>=2.0*kPi)phase-=2.0*kPi;
  const double target=std::min(peak,2.0)/(1.0+std::min(peak,2.0));
  gate=gatePole*gate+(1.0-gatePole)*target;
 }
 double biasValue() const noexcept {
  return bias ? amount*.00008*(.72*std::sin(phase)+.28*std::sin(phase*.606+0.4)) : 0.0;
 }
 // B: envelope-scaled, band-limited bias added to the saturator input of one channel.
 double haloValue(int ch,double x) noexcept {
  if(!halo)return 0.0;
  haloEnv[ch]=envPole*haloEnv[ch]+(1.0-envPole)*std::abs(x);
  if(haloEnv[ch]<1e-12)haloEnv[ch]=0.0;
  uint32_t s=haloRng[ch];s^=s<<13;s^=s>>17;s^=s<<5;haloRng[ch]=s;
  const double white=double(int32_t(s))/2147483648.0;
  const double y=haloB0*white+haloZ1[ch];
  haloZ1[ch]=haloB1*white-haloA1*y+haloZ2[ch];
  haloZ2[ch]=haloB2*white-haloA2*y;
  return amount*haloScale*haloKappa*haloEnv[ch]*haloNorm*y;
 }
 double residualValue(int ch) noexcept {
  if(!residual)return 0.0;
  uint32_t x=rng[ch];x^=x<<13;x^=x>>17;x^=x<<5;rng[ch]=x;
  const double white=double(int32_t(x))/2147483648.0;
  low[ch]=lowPole*low[ch]+(1.0-lowPole)*white;
  low2[ch]=lowPole*low2[ch]+(1.0-lowPole)*low[ch];
  wide[ch]=widePole*wide[ch]+(1.0-widePole)*white;
  const double colored=(lowPath?.005*low2[ch]:0.0)+.000035*wide[ch];
  const double y=colored-dcIn[ch]+dcPole*dc[ch];
  dcIn[ch]=colored;dc[ch]=y;
  return amount*gate*y;
 }
 double highpassAddition(int ch,double x) noexcept {
  const double y=x-outputIn[ch]+outputPole*outputHp[ch];
  outputIn[ch]=x;outputHp[ch]=y;return y;
 }
};
