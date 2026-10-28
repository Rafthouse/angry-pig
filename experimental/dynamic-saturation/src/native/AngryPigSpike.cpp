#include "AngryPigSpike.h"
#include <cstring>
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include <cmath>
#include <array>
static_assert(std::atomic<float>::is_always_lock_free && std::atomic<double>::is_always_lock_free,"Telemetry and parameter atomics must be lock free");
#if AP_DIAGNOSTICS
static std::atomic<AngryPigSpike*> registry[64];
static void APRegister(AngryPigSpike* p){for(auto& s:registry){AngryPigSpike* nil=nullptr;if(s.compare_exchange_strong(nil,p))return;}}
#endif
#include "IPlug_include_in_plug_src.h"
#ifdef OS_WIN
#include <windows.h>
#endif
#include <filesystem>
#include <fstream>
#include <cstdlib>
AngryPigSpike::AngryPigSpike(const InstanceInfo& info):Plugin(info,MakeConfig(4,1)) {
 for(auto& t:telemetry)t.store(0);
 GetParam(0)->InitDouble("ANGER",0,0,100,0.1,"%");
 GetParam(1)->InitDouble("COLOR",0,0,100,0.1,"%");
 GetParam(2)->InitDouble("GAIN",0,-24,18,0.1,"dB");
 GetParam(3)->InitBool("BYPASS",false);
 mEditorInitFunc=[this](){
#ifdef OS_WIN
 HMODULE mod=nullptr; GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&GetPluginFactory),&mod);
 wchar_t name[32768]; GetModuleFileNameW(mod,name,32768);
 auto index=std::filesystem::path(name).parent_path().parent_path()/"Resources"/"web"/"index.html";
 #if AP_DIAGNOSTICS
 if(auto path=std::getenv("AP_SPIKE_DIAG")){std::ofstream f(path,std::ios::app);f<<"index "<<index.string()<<"\n";}
#endif
 LoadFile(index.string().c_str(),nullptr);
#else
 LoadIndexHtml(__FILE__,GetBundleID());
#endif
 EnableScroll(false); };
 #if AP_TIMING
 timing.init();
 #endif
 {WP_SCOPE(timing,7,0,0,0);APFloatingPoint scope;engine.dynamic.configure();if(!engine.prepare())fault.store(-10);}
 MakePreset("Match Lab defaults",0.,0.,0.,0.);
 for(int i=0;i<4;++i)published[i].store(GetParam(i)->GetNormalized());
#if AP_DIAGNOSTICS
 APRegister(this);
 metrics.init(std::getenv("AP_SPIKE_METRICS"),reinterpret_cast<uintptr_t>(this));
#endif
}


bool AngryPigSpike::ConstrainEditorResize(int& w,int& h) const {
 const int oldW=w,oldH=h;
 // Stateless nearest-point projection onto width:height = 4:3.
 // A repeated OS proposal must have the same answer before/after onSize:
 // choosing an axis from the last committed size flips on 1px edge drags.
 double width=(16.*double(w)+12.*double(h))/25.;
 width=std::fmax(400.*editorDpiScale,std::fmin(1600.*editorDpiScale,width));
 w=static_cast<int>(std::lround(width));
 h=static_cast<int>(std::lround(w*3./4.));
 return w==oldW && h==oldH;
}
void AngryPigSpike::SetScreenScale(float scale) {
 if(!std::isfinite(scale)||scale<=0.f||scale==editorDpiScale)return;
 const double ratio=scale/editorDpiScale;
 editorDpiScale=scale;
 int w=static_cast<int>(std::lround(GetEditorWidth()*ratio));
 int h=static_cast<int>(std::lround(w*3./4.));
 auto* view=GetView();
 // Before attachment there is no platform view to resize. Once attached,
 // let the host commit physical dimensions through onSize (no feedback loop).
 if(view && view->HasFrame()) view->Resize(w,h);
 else SetEditorSize(w,h);
}

APValues AngryPigSpike::CurrentValues() const {std::lock_guard<std::recursive_mutex> guard(controlMutex);APValues f;if(feedback.read(f))cachedFeedback=f;APValues result=cachedFeedback;if(result.epoch!=controlValues.epoch)return controlValues;for(int i=0;i<4;i++)if(controlValues.version[i]>result.version[i]){result.v[i]=controlValues.v[i];result.version[i]=controlValues.version[i];}return result;}
void AngryPigSpike::OnReset(){WP_SCOPE(timing,2,GetSampleRate(),0,0);/* Framework hook: cold preparation belongs to inactive control lifecycle. */}
bool AngryPigSpike::PrepareColdIfNeeded(){
 if(componentActive.load()||engineLease.load()!=2)return false;
 const APValues target=CurrentValues();float params[4];for(int i=0;i<4;i++)params[i]=(float)target.v[i];
 const bool cold=!initialized.load()||initializedRate!=GetSampleRate()||fault.load()!=0||(!hasRun.load()&&std::memcmp(initializedParams,params,16)!=0);
 if(!cold)return true;
 APFloatingPoint fp;const int rc=engine.available?engine.reset((float)GetSampleRate(),params):-10;
 fault.store(rc);if(rc){initialized.store(false);return false;}
 initializedRate=GetSampleRate();std::memcpy(initializedParams,params,16);audioValues=target;feedback.write(audioValues);processError=0;hasRun.store(false);initialized.store(true);return true;
}
void AngryPigSpike::ProcessBlock(sample** in,sample** out,int frames) {
 APFloatingPoint scope;
 
 // Live recall never mutates pristine memory or invokes init.
 float blockPeaks[3]{};
 for(int at=0;at<frames;){WP_STAGE(3);int end=at+1;while(end<frames&&end-at<4096){bool same=true;for(int id=0;id<4;id++)if(plan.value[id][at]!=plan.value[id][end])same=false;if(!same)break;++end;}int n=end-at;
 WP_END(3);WP_STAGE(4);float params[4]={(float)(100*plan.value[0][at]),(float)(100*plan.value[1][at]),(float)(-24+42*plan.value[2][at]),(float)(plan.value[3][at]>=.5)};
 for(int ch=0;ch<2;ch++)for(int i=0;i<n;i++)scratchIn[ch][i]=(float)in[ch][at+i];
 WP_END(4);int rc=processError?processError:engine.process(scratchIn[0],scratchIn[1],scratchOut[0],scratchOut[1],n,params);if(rc){processError=rc;fault.store(rc);}
 WP_STAGE(9);for(int ch=0;ch<2;ch++)for(int i=0;i<n;i++)out[ch][at+i]=rc?0:(params[3]>=.5f?in[ch][at+i]:scratchOut[ch][i]);WP_END(9);WP_STAGE(10);
 if(!rc)for(int j=0;j<3;j++)blockPeaks[j]=std::fmax(blockPeaks[j],engine.meter[j]);
 WP_END(10);at=end;}
#if AP_DIAGNOSTICS
 for(int i=0;i<frames;++i){for(int id=0;id<4;++id)trace[i][id]=id==3?(plan.value[id][i]>=.5?1.:0.):plan.value[id][i];trace[i][4]=GetSamplePos()+i;trace[i][5]=GetTransportIsRunning()?1:0;}
#endif
 WP_STAGE(10);if(!processError){float packet[9];std::memcpy(packet,engine.meter.data(),36);for(int j=0;j<3;j++)packet[j]=blockPeaks[j];meterSequence.fetch_add(1,std::memory_order_acq_rel);for(int i=0;i<9;i++)telemetry[i].store(packet[i],std::memory_order_relaxed);meterSequence.fetch_add(1,std::memory_order_release);}
 WP_END(10);
}
void AngryPigSpike::OnWebContentLoaded(){
#if AP_DIAGNOSTICS
 if(auto path=std::getenv("AP_SPIKE_DIAG")){std::ofstream f(path,std::ios::app);f<<"content-loaded\n";}
 EvaluateJavaScript("JSON.stringify({url:location.href,title:document.title,body:document.body?.innerText?.slice(0,1000),scripts:[...document.scripts].map(x=>x.src),probe:typeof apProbe,bridge:typeof IPlugSendMsg,errors:window.apErrors})",[](const char* result){if(auto path=std::getenv("AP_SPIKE_DIAG")){std::ofstream f(path,std::ios::app);f<<"JS "<<(result?result:"null")<<"\n";}});
#endif
 ready=true;lastMeter=0;
 WebViewEditorDelegate::OnWebContentLoaded();
#if AP_DIAGNOSTICS
 if(std::getenv("AP_SPIKE_QA"))EvaluateJavaScript("window.apRunProbe()");
#endif
 for(int i=0;i<4;++i)SendParameterValueFromDelegate(i,getParamNormalized(i),true);
}
bool AngryPigSpike::OnCanNavigateToURL(const char* url){
#ifdef OS_WIN
return std::string(url).rfind("https://iplug.example/",0)==0;
#else
return std::string(url).rfind("file://",0)==0; // Mac bundle loading: not runtime-qualified in this spike.
#endif
}
bool AngryPigSpike::OnMessage(int tag,int,int size,const void* data){
#if AP_DIAGNOSTICS
 if(tag==901 && size>0 && size<65536){if(const char* path=std::getenv("AP_SPIKE_LOG")){std::ofstream f(path,std::ios::app);f.write((const char*)data,size);f<<"\n";}}
#endif
 return false;
}

void AngryPigSpike::SendParameterValueFromDelegate(int index,double value,bool normalized){if(!ready)return;if(!normalized)value=GetParam(index)->ToNormalized(value);char script[128];std::snprintf(script,sizeof(script),"SPVFD(%d,%.17g)",index,value);EvaluateJavaScript(script);}

AngryPigSpike::~AngryPigSpike(){
#if AP_DIAGNOSTICS
metrics.flush();
for(auto& s:registry)if(s.load()==this)s.store(nullptr);
#endif
}

void AngryPigSpike::OnParamChange(int id){dirty.fetch_or(15u);}
void AngryPigSpike::OnParamChangeUI(int id,EParamSource source){if(id<0||id>=4||source==kDelegate)return;std::lock_guard<std::recursive_mutex> guard(controlMutex);auto current=CurrentValues();controlValues.v[id]=GetParam(id)->Value();controlValues.version[id]++;controls.write(controlValues);dirty.fetch_or(15u);}
Steinberg::tresult PLUGIN_API AngryPigSpike::setParamNormalized(Steinberg::Vst::ParamID id,double value){if(id>=4||!std::isfinite(value)||value<0||value>1)return Steinberg::kResultFalse;std::lock_guard<std::recursive_mutex> guard(controlMutex);GetParam(id)->SetNormalized(value);OnParamChangeUI(id,kHost);SetVST3ParamNormalized(id,value);return Steinberg::kResultOk;}
Steinberg::Vst::ParamValue PLUGIN_API AngryPigSpike::getParamNormalized(Steinberg::Vst::ParamID id){if(id>=4)return 0;auto x=CurrentValues();return GetParam(id)->ToNormalized(x.v[id]);}
bool AngryPigSpike::SerializeState(IByteChunk& chunk)const{auto x=CurrentValues();return chunk.PutBytes(x.v,32)>0;}
Steinberg::tresult PLUGIN_API AngryPigSpike::getState(Steinberg::IBStream* stream){if(!stream)return Steinberg::kInvalidArgument;auto x=CurrentValues();char b[36];std::memcpy(b,x.v,32);int mirror=x.v[3]>=.5;std::memcpy(b+32,&mirror,4);Steinberg::int32 used=0;while(used<36){Steinberg::int32 n=0;auto rc=stream->write(b+used,36-used,&n);if(n<=0||n>36-used||rc!=Steinberg::kResultOk)return Steinberg::kResultFalse;used+=n;}return Steinberg::kResultOk;}
Steinberg::tresult PLUGIN_API AngryPigSpike::setState(Steinberg::IBStream* stream){WP_SCOPE(timing,6,0,0,0);if(!stream)return Steinberg::kInvalidArgument;char b[37];Steinberg::int32 used=0;while(used<37){Steinberg::int32 n=0;auto rc=stream->read(b+used,37-used,&n);if(n<0||n>37-used)return Steinberg::kResultFalse;if(!n)break;used+=n;if(rc!=Steinberg::kResultOk)break;}if(used!=36)return Steinberg::kResultFalse;IByteChunk chunk;chunk.PutBytes(b,36);return UnserializeState(chunk,0)==32?Steinberg::kResultOk:Steinberg::kResultFalse;}
void AngryPigSpike::OnIdle(){
#if AP_DIAGNOSTICS
metrics.flush();
#endif
const unsigned bits=dirty.exchange(0);if(bits){std::lock_guard<std::recursive_mutex> guard(controlMutex);auto current=CurrentValues();for(int i=0;i<4;i++){GetParam(i)->Set(current.v[i]);double v=GetParam(i)->ToNormalized(current.v[i]);SetVST3ParamNormalized(i,v);SendParameterValueFromDelegate(i,v,true);}}
 if(ready){unsigned seq=meterSequence.load(std::memory_order_acquire);if(!(seq&1)&&seq!=lastMeter){float packet[9];for(int i=0;i<9;i++)packet[i]=telemetry[i].load(std::memory_order_relaxed);if(meterSequence.load(std::memory_order_acquire)==seq){lastMeter=seq;char js[512];std::snprintf(js,sizeof(js),"window.__vstOnProcessorBuffers({meter_v2:[%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g]})",packet[0],packet[1],packet[2],packet[3],packet[4],packet[5],packet[6],packet[7],packet[8]);EvaluateJavaScript(js);}}char status[96];std::snprintf(status,sizeof(status),"window.apNativeFault=%d",fault.load());EvaluateJavaScript(status);}}
int AngryPigSpike::UnserializeState(const IByteChunk& chunk,int pos){if(pos!=0||chunk.Size()!=36)return -1;double values[4];for(int i=0;i<4;++i){pos=chunk.Get(&values[i],pos);if(pos<0||!std::isfinite(values[i])||values[i]<GetParam(i)->GetMin()||values[i]>GetParam(i)->GetMax()||(i==3&&values[i]!=0&&values[i]!=1))return -1;}std::lock_guard<std::recursive_mutex> guard(controlMutex);controlValues.epoch++;for(int i=0;i<4;i++){controlValues.v[i]=values[i];controlValues.version[i]++;}controls.write(controlValues);dirty.fetch_or(15u);return pos;}
void AngryPigSpike::OnMessageFromWebView(const char* message){auto j=nlohmann::json::parse(message,nullptr,false);if(j.is_discarded()||!j.is_object()||!j.contains("msg")||!j["msg"].is_string())return;const auto msg=j["msg"].get<std::string>();if(msg=="SPVFUI"||msg=="BPCFUI"||msg=="EPCFUI"){if(!j.contains("paramIdx")||!j["paramIdx"].is_number_integer())return;int id=j["paramIdx"];if(id<0||id>=4)return;
 if(msg=="BPCFUI"){if(!gestures[id]){BeginInformHostOfParamChangeFromUI(id);gestures[id]=true;}return;}
 if(msg=="EPCFUI"){if(gestures[id]){EndInformHostOfParamChangeFromUI(id);gestures[id]=false;}return;}
 if(!j.contains("value")||!j["value"].is_number())return;double v=j["value"];if(!std::isfinite(v)||v<0||v>1)return;bool implicit=!gestures[id];if(implicit)BeginInformHostOfParamChangeFromUI(id);{std::lock_guard<std::recursive_mutex> guard(controlMutex);SendParameterValueFromUI(id,v);}if(implicit)EndInformHostOfParamChangeFromUI(id);return;}
 WebViewEditorDelegate::OnMessageFromWebView(message);}
void AngryPigSpike::CloseWindow(){for(int i=0;i<4;++i)if(gestures[i]){EndInformHostOfParamChangeFromUI(i);gestures[i]=false;}ready=false;WebViewEditorDelegate::CloseWindow();}
Steinberg::tresult PLUGIN_API AngryPigSpike::process(Steinberg::Vst::ProcessData& data){
 APEngineLease lease(engineLease,1);if(!lease)return Steinberg::kResultFalse;
 WP_SCOPE(timing,1,GetSampleRate(),data.numSamples,audioValues.epoch);
 APFloatingPoint fpScope;
#if AP_DIAGNOSTICS
 metricReset=false;APMeasure measure(metrics,GetSampleRate(),data.numSamples,metricReset);
#endif
using namespace Steinberg;using namespace Steinberg::Vst;
 traceFrames=0;if(data.numSamples<0||data.numSamples>APPlan::MaxFrames||data.numInputs>1||data.numOutputs>1)return kInvalidArgument;
 WP_STAGE(1);for(int id=0;id<4;++id)plan.count[id]=0;APValues incoming; if(controls.read(incoming)){if(incoming.epoch!=audioValues.epoch){audioValues=incoming;}else for(int id=0;id<4;id++)if(incoming.version[id]>audioValues.version[id]){audioValues.v[id]=incoming.v[id];audioValues.version[id]=incoming.version[id];}}double initial[4];for(int id=0;id<4;id++)initial[id]=GetParam(id)->ToNormalized(audioValues.v[id]);
 WP_END(1);
#if AP_TIMING
 if(wp_frame){wp_frame->r.epoch=audioValues.epoch;wp_frame->r.fault=fault.load();}
#endif
 WP_STAGE(2);if(data.inputParameterChanges){int queues=data.inputParameterChanges->getParameterCount();if(queues<0||queues>4)return kInvalidArgument;bool seen[4]{};for(int q=0;q<queues;++q){auto* queue=data.inputParameterChanges->getParameterData(q);if(!queue)return kInvalidArgument;unsigned id=queue->getParameterId();if(id>=4||seen[id])return kInvalidArgument;seen[id]=true;int n=queue->getPointCount();if(n<0||n>APPlan::MaxPoints)return kInvalidArgument;plan.count[id]=n;wp_count(0,n);for(int j=0;j<n;++j){int32 x;double v;if(queue->getPoint(j,x,v)!=kResultTrue)return kInvalidArgument;plan.points[id][j]={x,v};}}}
 WP_END(2);WP_STAGE(3);if(!plan.build(data.numSamples,initial))return kInvalidArgument;WP_END(3);
 #if AP_DIAGNOSTICS
 metricReset=false;
#endif


 processError=fault.load();
 if(data.numSamples==0){for(int id=0;id<4;id++)if(plan.count[id]){audioValues.v[id]=GetParam(id)->FromNormalized(plan.final[id]);}feedback.write(audioValues);dirty.fetch_or(15u);return kResultOk;}
 WP_STAGE(4);if(data.numInputs!=1||data.numOutputs!=1||!data.inputs||!data.outputs||data.inputs[0].numChannels!=2||data.outputs[0].numChannels!=2)return kInvalidArgument;
 for(int ch=0;ch<2;ch++){if(data.symbolicSampleSize==kSample32){if(!data.inputs[0].channelBuffers32||!data.outputs[0].channelBuffers32||!data.inputs[0].channelBuffers32[ch]||!data.outputs[0].channelBuffers32[ch])return kInvalidArgument;}else if(data.symbolicSampleSize==kSample64){if(!data.inputs[0].channelBuffers64||!data.outputs[0].channelBuffers64||!data.inputs[0].channelBuffers64[ch]||!data.outputs[0].channelBuffers64[ch])return kInvalidArgument;}else return kInvalidArgument;}
 if(data.numSamples>GetBlockSize())return kInvalidArgument;
 for(int ch=0;ch<2;ch++)for(int i=0;i<data.numSamples;i++){double v=data.symbolicSampleSize==kSample32?data.inputs[0].channelBuffers32[ch][i]:data.inputs[0].channelBuffers64[ch][i];if(!std::isfinite(v)||std::fabs(v)>3.4028234663852886e38){for(int c=0;c<2;c++)for(int j=0;j<data.numSamples;j++){if(data.symbolicSampleSize==kSample32)data.outputs[0].channelBuffers32[c][j]=0;else data.outputs[0].channelBuffers64[c][j]=0;}return kInvalidArgument;}}
 WP_END(4);if(!initialized.load()){for(int ch=0;ch<2;ch++)for(int i=0;i<data.numSamples;i++){if(data.symbolicSampleSize==kSample32)data.outputs[0].channelBuffers32[ch][i]=0;else data.outputs[0].channelBuffers64[ch][i]=0;}return kResultFalse;}auto block=data;block.inputParameterChanges=nullptr;offset=0;auto result=IPlugVST3::process(block);if(result!=kResultOk)return result;
 for(int id=0;id<4;id++)if(plan.count[id]){audioValues.v[id]=GetParam(id)->FromNormalized(id==3?(plan.final[id]>=.5?1.:0.):plan.final[id]);}
 feedback.write(audioValues);dirty.fetch_or(15u);data.outputs[0].silenceFlags=0;

 if(!processError)hasRun.store(true);traceFrames=data.numSamples;return processError?kResultFalse:kResultOk;
}
#if AP_DIAGNOSTICS
extern "C" __declspec(dllexport) double APReadTrace(void* component,int sample,int id){for(auto& slot:registry){auto* p=slot.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component&&sample>=0&&sample<p->traceFrames&&id>=0&&id<6)return p->trace[sample][id];}return -999;}

extern "C" __declspec(dllexport) void APTestUIMessage(void* component,const char* message){for(auto& slot:registry){auto* p=slot.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component){p->OnMessageFromWebView(message);return;}}}

extern "C" __declspec(dllexport) int APDbgCount(void* component){for(auto& slot:registry){auto*p=slot.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component)return p->engine.dbgCount;}return -1;}
extern "C" __declspec(dllexport) double APReadDbg(void* component,int which,int index){for(auto& slot:registry){auto*p=slot.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component&&index>=0&&index<p->engine.dbgCount)return which==0?p->engine.dbgEnv[index]:p->engine.dbgRed[index];}return -999;}
extern "C" __declspec(dllexport) float APReadDSP(void* component,int field){for(auto& slot:registry){auto*p=slot.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component){if(field==-1)return (float)p->fault.load();if(field==-2)return 0;if(field>=0&&field<9)return p->engine.meter[field];return 0;}}return -999;}
extern "C" __declspec(dllexport) unsigned long long WP3AppliedEpoch(void* component){for(auto&slot:registry){auto*p=slot.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component){APValues x;if(p->feedback.read(x))return x.epoch;}}return ~0ull;}
extern "C" __declspec(dllexport) void APInjectTrap(void* component){for(auto& slot:registry){auto*p=slot.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component)p->engine.available=false;}}

extern "C" __declspec(dllexport) void APTestCapture(void*component){for(auto& slot:registry){auto*p=slot.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component)p->EvaluateJavaScript("window.apCapture && window.apCapture(window.apNativeFault?'native-fault':'native-meter-on')");}}
static char gTestEvalResult[4096]={};
extern "C" __declspec(dllexport) void APTestEval(void*component,const char* script){for(auto& slot:registry){auto*p=slot.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component)p->EvaluateJavaScript(script,[](const char* result){std::snprintf(gTestEvalResult,sizeof(gTestEvalResult),"%s",result?result:"");});}}
extern "C" __declspec(dllexport) const char* APTestEvalResult(){return gTestEvalResult;}

#endif

Steinberg::tresult PLUGIN_API AngryPigSpike::setupProcessing(Steinberg::Vst::ProcessSetup& s){
 #if AP_TIMING
 timing.setup.fetch_add(1);
 #endif
 WP_SCOPE(timing,3,s.sampleRate,s.maxSamplesPerBlock,0);
 if(componentActive.load()||!std::isfinite(s.sampleRate)||s.sampleRate<8000||s.sampleRate>384000||s.maxSamplesPerBlock<=0||s.maxSamplesPerBlock>APPlan::MaxFrames)return Steinberg::kInvalidArgument;
 std::lock_guard<std::recursive_mutex> lock(controlMutex);APEngineLease lease(engineLease,2);if(!lease)return Steinberg::kResultFalse;
 auto result=IPlugVST3::setupProcessing(s);if(result!=Steinberg::kResultOk)return result;
 return PrepareColdIfNeeded()?Steinberg::kResultOk:Steinberg::kResultFalse;
}
Steinberg::tresult PLUGIN_API AngryPigSpike::setActive(Steinberg::TBool state){
 WP_SCOPE(timing,state?9:10,GetSampleRate(),0,0);
 std::lock_guard<std::recursive_mutex> lock(controlMutex);APEngineLease lease(engineLease,2);if(!lease)return Steinberg::kResultFalse;
 if(state&&!componentActive.load()&&!PrepareColdIfNeeded())return Steinberg::kResultFalse;
 auto result=IPlugVST3::setActive(state);if(result==Steinberg::kResultOk)componentActive.store(state!=0);return result;
}
Steinberg::tresult PLUGIN_API AngryPigSpike::setProcessing(Steinberg::TBool state){WP_SCOPE(timing,state?5:4,GetSampleRate(),0,0);if(state&&(!initialized.load()||fault.load()!=0))return Steinberg::kResultFalse;return IPlugVST3::setProcessing(state);}
#if AP_TIMING
extern "C" __declspec(dllexport) void WP1Dump(void*component,const char*path){for(auto&s:registry){auto*p=s.load();if(p&&static_cast<Steinberg::Vst::IComponent*>(p)==component)p->timing.dump(path);}}
#endif

#if AP_TIMING
extern "C" __declspec(dllexport) void WP1Calibrate(void*component,const char*path){for(auto&s:registry){auto*p=s.load();if(!p||static_cast<Steinberg::Vst::IComponent*>(p)!=component)continue;FILE*f=fopen(path,"w");if(!f)return;fprintf(f,"{\"frequency\":%llu,\"emptyEntryTicks\":[",p->timing.freq);for(int trial=0;trial<20000;trial++){WPFrame frame;auto*saved=wp_frame;wp_frame=&frame;auto begin=wp_now();wp_begin(3);wp_end(3);wp_begin(4);wp_end(4);wp_begin(7);wp_end(7);wp_begin(6);wp_end(6);wp_begin(7);wp_end(7);wp_begin(8);wp_end(8);wp_begin(7);wp_end(7);wp_begin(9);wp_end(9);wp_begin(10);wp_end(10);auto end=wp_now();wp_frame=saved;fprintf(f,"%s%llu",trial?",":"",end-begin);}fprintf(f,"]}");fclose(f);}}
#endif
