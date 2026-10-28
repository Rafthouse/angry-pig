#pragma once

#include "APMatchTables.h"
#include "APDynamicSaturation.h"
#include <algorithm>
#include <array>
#include <cmath>

// An isolated, measurement-derived behavioral model. The four dB
// compressor knee is the intentionally preserved Angry Pig distinction.
struct APDspEngine {
 struct Biquad {
  double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
  void set(double c0,double c1,double c2,double c3,double c4) noexcept {b0=c0;b1=c1;b2=c2;a1=c3;a2=c4;}
  void clear() noexcept {z1=z2=0;}
  double run(double x) noexcept {const double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y;}
 };
 Biquad hp[2][2],colorFilter[2];
 std::array<float,9> meter{};
 double rate=48000,smoothedFat=0,smoothedGain=0,envelope=0,lastColor=-1;
 double parameterPole=0,attackPole=0,releasePole=0,meterPole=0;
 bool available=false;
 APDynamicSaturation dynamic;
#if AP_DIAGNOSTICS
 static constexpr int DbgMax=1<<17;double dbgEnv[DbgMax]{};double dbgRed[DbgMax]{};int dbgCount=0; // test-only per-sample envelope and reduction
#endif
 static constexpr double pi=3.14159265358979323846;
 // RC1 selected DSP model: hard knee (width 0). Approved r3 binary uses 4.0 and is unchanged.
 static constexpr double kneeDb=0.0;
 static constexpr double peakBandwidthOct=1.8930115;
 static constexpr double baseGain=1.1220184543019633;
 static constexpr double thresholds[4]={0.251188643150958,0.5011872336272722,1.,1.9952623149688795};

 static void setHighpass(Biquad& b,double fs,double fc,double q) noexcept {
  const double w=2*pi*fc/fs,co=std::cos(w),alpha=std::sin(w)/(2*q),d=1+alpha;
  b.set((1+co)/(2*d),-(1+co)/d,(1+co)/(2*d),-2*co/d,(1-alpha)/d);
 }
 static double colorCenter(double c) noexcept {
  using namespace APMatchTables;
  constexpr int last=int(sizeof(ColorKnots)/sizeof(ColorKnots[0]))-1;
  c=std::clamp(c,0.0,1.0);
  int segment=0;
  while(segment<last-1&&c>ColorKnots[segment+1])++segment;
  const double x=c-ColorKnots[segment];
  const auto& p=ColorPolynomial[segment];
  return ((p[0]*x+p[1])*x+p[2])*x+p[3];
 }
 void setColor(double c) noexcept {
  if(c==lastColor)return;
  lastColor=c;
  const double fc=colorCenter(c),w=2*pi*fc/rate,sw=std::sin(w),cw=std::cos(w);
  const double A=std::pow(10.,8*c/40.);
  const double alpha=sw*std::sinh((std::log(2.)/2)*peakBandwidthOct*w/sw);
  const double d=1+alpha/A;
  for(auto& b:colorFilter)b.set((1+alpha*A)/d,-2*cw/d,(1-alpha*A)/d,-2*cw/d,(1-alpha/A)/d);
 }
 static double saturate(double v) noexcept {
  using namespace APMatchTables;
  const double ab=std::abs(v);
  if(ab<=ClipStart)return v;
  if(ab>=ClipEnd)return std::copysign(ClipCeiling,v);
  const double pos=(ab-ClipStart)*(ClipSegments/(ClipEnd-ClipStart));
  const auto i=static_cast<int>(pos);
  const double y=ClipCurve[i]+(ClipCurve[i+1]-ClipCurve[i])*(pos-i);
  return std::copysign(y,v);
 }
 bool prepare() noexcept {available=true;return true;}
 // LF high-pass fits per sample rate (fc1, Q1, fc2, Q2), each fitted to measured REF sweeps at that rate.
 // Held-out check (22-175 Hz), max error: 44.1 kHz 0.0005 dB, 48 kHz 0.0015 dB, 88.2 kHz 0.0007 dB, 96 kHz 0.0017 dB, 192 kHz 0.0097 dB (22 Hz). Other rates use the nearest set.
 static const double* hpFit(double fs) noexcept {
  static constexpr double fit[5][4]={{29.2538,.6538,19.4956,.6521},{29.9297,.6687,20.1137,.6732},{29.8991,.6677,19.6994,.6599},{19.7640,.6606,30.0598,.6723},{19.7640,.6602,29.8143,.6669}};
  return fs<46000.0?fit[0]:fs<72000.0?fit[1]:fs<92000.0?fit[2]:fs<144000.0?fit[3]:fit[4];
 }
 int reset(float fs,const float params[4]) noexcept {
  if(!available||!params||!std::isfinite(fs)||fs<8000||fs>384000)return -1;
  rate=fs;
  const double* lf=hpFit(rate);
  for(int channel=0;channel<2;++channel){
   setHighpass(hp[channel][0],rate,lf[0],lf[1]);
   setHighpass(hp[channel][1],rate,lf[2],lf[3]);
   hp[channel][0].clear();hp[channel][1].clear();colorFilter[channel].clear();
  }
  lastColor=-1;
  smoothedFat=std::clamp(double(params[0])/100.,0.,1.);
  smoothedGain=std::clamp(double(params[2]),-24.,18.);
  setColor(std::clamp(double(params[1])/100.,0.,1.));
  parameterPole=std::exp(-1/(rate*.006));
  attackPole=std::exp(-1/(rate*.0024514));
  releasePole=std::exp(-1/(rate*.03024481));
  meterPole=std::exp(-1/(rate*.18));
  envelope=0;meter.fill(0);meter[8]=2;
  dynamic.reset(rate);
#if AP_DIAGNOSTICS
 dbgCount=0;
#endif
  return 0;
 }
 int process(const float* left,const float* right,float* outLeft,float* outRight,int n,const float params[4]) noexcept {
  if(!available||!left||!right||!outLeft||!outRight||!params||n<0||n>4096)return -2;
  for(int j=0;j<4;++j)if(!std::isfinite(params[j]))return -3;
  if(params[0]<0||params[0]>100||params[1]<0||params[1]>100||params[2]<-24||params[2]>18||params[3]<0||params[3]>1)return -4;
  const double targetFat=params[0]/100.,targetGain=params[2];
  setColor(params[1]/100.);
  meter[0]=meter[1]=meter[2]=0;
  for(int i=0;i<n;++i){
   if(!std::isfinite(left[i])||!std::isfinite(right[i]))return -5;
   smoothedFat=parameterPole*smoothedFat+(1-parameterPole)*targetFat;
   smoothedGain=parameterPole*smoothedGain+(1-parameterPole)*targetGain;
   const double linearGain=std::pow(10.,(39*smoothedFat+smoothedGain)/20.)*baseGain;
   const double pre[2]={
    colorFilter[0].run(hp[0][1].run(hp[0][0].run(left[i])))*linearGain,
    colorFilter[1].run(hp[1][1].run(hp[1][0].run(right[i])))*linearGain
   };
   const double detector=std::max(std::abs(pre[0]),std::abs(pre[1]));
   const double coefficient=detector>envelope?attackPole:releasePole;
   envelope=coefficient*envelope+(1-coefficient)*detector;
   const double overDb=20*std::log10(std::max(envelope,1e-30))-5*smoothedFat;
   double reductionDb=0;
   if(overDb>=kneeDb/2)reductionDb=.9*overDb;
   else if(overDb>-kneeDb/2){const double t=overDb+kneeDb/2;reductionDb=.9*t*t/(2*kneeDb);}
#if AP_DIAGNOSTICS
 if(dbgCount<DbgMax){dbgEnv[dbgCount]=envelope;dbgRed[dbgCount]=reductionDb;++dbgCount;}
#endif
   const double compressorGain=std::pow(10.,-reductionDb/20.);
   const double shaped[2]={pre[0]*compressorGain,pre[1]*compressorGain};
   const double peak=std::max(std::abs(shaped[0]),std::abs(shaped[1]));
   const bool clipEvent=peak>.90;
   for(int ch=0;ch<2;++ch)meter[ch]=std::max(meter[ch],float(std::abs(shaped[ch])));
   if(clipEvent)meter[2]=1;
   for(int j=0;j<4;++j)meter[3+j]=peak>=thresholds[j]?1.f:float(meter[3+j]*meterPole);
   meter[7]=clipEvent?1.f:float(meter[7]*meterPole);
   if(!dynamic.enabled()) {
    outLeft[i]=params[3]>=.5f?left[i]:float(saturate(shaped[0]));
    outRight[i]=params[3]>=.5f?right[i]:float(saturate(shaped[1]));
   } else {
    dynamic.advance(peak);
    const double bias=dynamic.biasValue();
    // Center the transfer at zero: a silent input produces exact digital silence.
    const double center=saturate(bias);
    const double baseL=saturate(shaped[0]),baseR=saturate(shaped[1]);
    const double haloL=dynamic.haloValue(0,shaped[0]),haloR=dynamic.haloValue(1,shaped[1]);
    const double extraL=saturate(shaped[0]+haloL+bias)-center+dynamic.residualValue(0)-baseL;
    const double extraR=saturate(shaped[1]+haloR+bias)-center+dynamic.residualValue(1)-baseR;
    outLeft[i]=params[3]>=.5f?left[i]:float(baseL+dynamic.highpassAddition(0,extraL));
    outRight[i]=params[3]>=.5f?right[i]:float(baseR+dynamic.highpassAddition(1,extraR));
   }
  }
  meter[8]=2;
  return 0;
 }
};
