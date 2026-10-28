#pragma once
#include "IPlug_include_in_plug_hdr.h"
#include "APAutomation.h"
#include <atomic>
#include <mutex>
#include "APSnapshot.h"
#include "APMetrics.h"
#include "APTiming.h"
#include "APDspEngine.h"
#include "APEngineLease.h"
#include "APFloatingPoint.h"
using namespace iplug;
class AngryPigSpike final : public Plugin {
public:
 // VST3 dimensions are physical pixels; the frozen DOM scene is 800 x 600.
 float editorDpiScale=1.f;
 bool ConstrainEditorResize(int& w,int& h) const override;
 void SetScreenScale(float scale) override;
 AngryPigSpike(const InstanceInfo& info);
 ~AngryPigSpike();
 Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;
 bool SerializeState(IByteChunk& chunk) const override;
 Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream*) override;
 Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream*) override;
 Steinberg::tresult PLUGIN_API setParamNormalized(Steinberg::Vst::ParamID,double) override;
 Steinberg::Vst::ParamValue PLUGIN_API getParamNormalized(Steinberg::Vst::ParamID) override;
 APValues CurrentValues() const;
#if AP_DIAGNOSTICS
 APMetrics metrics;bool metricReset=false;
#endif
 APBank controls,feedback;APValues controlValues,audioValues;mutable APValues cachedFeedback;mutable std::recursive_mutex controlMutex;
 #if AP_TIMING
 WPTiming timing;
 #endif
 Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup&) override;
 Steinberg::tresult PLUGIN_API setProcessing(Steinberg::TBool) override;
 Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool) override;
 bool PrepareColdIfNeeded(); // caller owns controlMutex and engineLease=2; inactive UI lifecycle only
 std::atomic<unsigned> engineLease{0};
 std::atomic<bool> initialized{false},hasRun{false},componentActive{false};
 double initializedRate=0;float initializedParams[4]{};

 void OnReset() override;
 void OnParamChange(int id) override;
 void OnParamChangeUI(int id,EParamSource source=kUnknown) override;
 void OnIdle() override;
 int UnserializeState(const IByteChunk& chunk,int pos) override;
 void OnMessageFromWebView(const char* message) override;
 void CloseWindow() override;
 APDspEngine engine;std::atomic<int> fault{0};int processError=0;std::atomic<float> telemetry[9];std::atomic<unsigned> meterSequence{0};unsigned lastMeter=0;float scratchIn[2][4096]{},scratchOut[2][4096]{};
 APPlan plan;
#if AP_DIAGNOSTICS
 double trace[APPlan::MaxFrames][6]{};
#endif
int traceFrames=0,offset=0;bool ready=false,gestures[4]{};std::atomic<unsigned> dirty{0};std::atomic<double> published[4];

 void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
 void OnWebContentLoaded() override;
 void SendParameterValueFromDelegate(int index,double value,bool normalized) override;
 bool OnMessage(int tag,int ctrl,int size,const void* data) override;
 bool OnCanNavigateToURL(const char* url) override;
};
