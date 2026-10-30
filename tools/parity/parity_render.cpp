// Offline render harness for the Angry Pig VST3 and AUv2 parity check.
// Test code only: it loads an installed plug-in, feeds a WAV file through it in fixed
// blocks, and writes a float32 WAV. It does not touch the plug-in source.
//
// usage: parity_render <vst3|au> <plugin> <in.wav> <out.wav> <anger 0..100> <color 0..100>
//                      <gain -36..36> <bypass 0|1> <none|anger-ramp>
// <plugin>: path to the .vst3 bundle (VST3) or "-" (AU, found by type aufx / APm1 / APLb).
// AP_PARITY_BLOCK sets the block size (default 512).|au> <in.wav> <out.wav> <anger 0..100> <color 0..100>
//                      <gain -24..18> <bypass 0|1> <none|anger-ramp>
#if defined(__APPLE__)
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/vst/vstspeaker.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

static int kBlock = 512;
static std::string kVst3Path;

struct Wav {
    double sampleRate = 48000;
    int channels = 2;
    std::vector<float> ch[2];
};

static bool readWav(const char* path, Wav& w) {
    FILE* f = std::fopen(path, "rb");
    if (!f) return false;
    std::vector<uint8_t> raw;
    uint8_t buf[65536];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) raw.insert(raw.end(), buf, buf + n);
    std::fclose(f);
    if (raw.size() < 12 || std::memcmp(raw.data(), "RIFF", 4) || std::memcmp(raw.data() + 8, "WAVE", 4)) return false;
    size_t p = 12;
    int fmtTag = 0, bits = 0;
    const uint8_t* data = nullptr;
    size_t dataLen = 0;
    while (p + 8 <= raw.size()) {
        uint32_t len;
        std::memcpy(&len, raw.data() + p + 4, 4);
        const uint8_t* body = raw.data() + p + 8;
        if (!std::memcmp(raw.data() + p, "fmt ", 4)) {
            uint16_t tag, chans;
            uint32_t rate;
            uint16_t b;
            std::memcpy(&tag, body, 2);
            std::memcpy(&chans, body + 2, 2);
            std::memcpy(&rate, body + 4, 4);
            std::memcpy(&b, body + 14, 2);
            fmtTag = tag;
            w.channels = chans;
            w.sampleRate = rate;
            bits = b;
        } else if (!std::memcmp(raw.data() + p, "data", 4)) {
            data = body;
            dataLen = len;
        }
        p += 8 + len + (len & 1);
    }
    if (!data || w.channels != 2) return false;
    int bytes = bits / 8;
    size_t frames = dataLen / (bytes * 2);
    for (int c = 0; c < 2; ++c) w.ch[c].assign(frames, 0.f);
    for (size_t i = 0; i < frames; ++i)
        for (int c = 0; c < 2; ++c) {
            const uint8_t* s = data + (i * 2 + c) * bytes;
            if (fmtTag == 3 && bits == 32) { float v; std::memcpy(&v, s, 4); w.ch[c][i] = v; }
            else if (fmtTag == 1 && bits == 16) { int16_t v; std::memcpy(&v, s, 2); w.ch[c][i] = v / 32768.f; }
            else return false;
        }
    return true;
}

static bool writeWav(const char* path, const std::vector<float> ch[2], double rate) {
    FILE* f = std::fopen(path, "wb");
    if (!f) return false;
    uint32_t frames = (uint32_t)ch[0].size();
    uint32_t dataLen = frames * 2 * 4;
    auto w32 = [&](uint32_t v) { std::fwrite(&v, 4, 1, f); };
    auto w16 = [&](uint16_t v) { std::fwrite(&v, 2, 1, f); };
    std::fwrite("RIFF", 1, 4, f); w32(36 + dataLen); std::fwrite("WAVE", 1, 4, f);
    std::fwrite("fmt ", 1, 4, f); w32(16); w16(3); w16(2); w32((uint32_t)rate); w32((uint32_t)rate * 8); w16(8); w16(32);
    std::fwrite("data", 1, 4, f); w32(dataLen);
    for (uint32_t i = 0; i < frames; ++i)
        for (int c = 0; c < 2; ++c) std::fwrite(&ch[c][i], 4, 1, f);
    std::fclose(f);
    return true;
}

// ----- minimal host objects -------------------------------------------------

class HostApp : public IHostApplication {
public:
    tresult PLUGIN_API getName(String128 name) override {
        const char* s = "AngryPigParityHost";
        for (int i = 0; i < 128; ++i) name[i] = i < (int)std::strlen(s) ? (TChar)s[i] : 0;
        return kResultOk;
    }
    tresult PLUGIN_API createInstance(TUID, TUID, void** obj) override { *obj = nullptr; return kNotImplemented; }
    tresult PLUGIN_API queryInterface(const TUID iid, void** obj) override {
        if (FUnknownPrivate::iidEqual(iid, FUnknown::iid) || FUnknownPrivate::iidEqual(iid, IHostApplication::iid)) {
            *obj = this;
            return kResultOk;
        }
        *obj = nullptr;
        return kNoInterface;
    }
    uint32 PLUGIN_API addRef() override { return 1; }
    uint32 PLUGIN_API release() override { return 1; }
};

class ParamQueue : public IParamValueQueue {
public:
    ParamID id = 0;
    int32 count = 0;
    int32 offsets[4] = {};
    double values[4] = {};
    ParamID PLUGIN_API getParameterId() override { return id; }
    int32 PLUGIN_API getPointCount() override { return count; }
    tresult PLUGIN_API getPoint(int32 index, int32& sampleOffset, ParamValue& value) override {
        if (index < 0 || index >= count) return kResultFalse;
        sampleOffset = offsets[index];
        value = values[index];
        return kResultTrue;
    }
    tresult PLUGIN_API addPoint(int32 sampleOffset, ParamValue value, int32& index) override {
        if (count >= 4) return kResultFalse;
        offsets[count] = sampleOffset;
        values[count] = value;
        index = count++;
        return kResultTrue;
    }
    tresult PLUGIN_API queryInterface(const TUID iid, void** obj) override {
        if (FUnknownPrivate::iidEqual(iid, FUnknown::iid) || FUnknownPrivate::iidEqual(iid, IParamValueQueue::iid)) {
            *obj = this;
            return kResultOk;
        }
        *obj = nullptr;
        return kNoInterface;
    }
    uint32 PLUGIN_API addRef() override { return 1; }
    uint32 PLUGIN_API release() override { return 1; }
};

class ParamChanges : public IParameterChanges {
public:
    ParamQueue queues[4];
    int32 used = 0;
    int32 PLUGIN_API getParameterCount() override { return used; }
    IParamValueQueue* PLUGIN_API getParameterData(int32 index) override {
        return index >= 0 && index < used ? &queues[index] : nullptr;
    }
    IParamValueQueue* PLUGIN_API addParameterData(const ParamID& id, int32& index) override {
        for (int32 i = 0; i < used; ++i)
            if (queues[i].id == id) { index = i; return &queues[i]; }
        if (used >= 4) return nullptr;
        queues[used].id = id;
        queues[used].count = 0;
        index = used;
        return &queues[used++];
    }
    tresult PLUGIN_API queryInterface(const TUID iid, void** obj) override {
        if (FUnknownPrivate::iidEqual(iid, FUnknown::iid) || FUnknownPrivate::iidEqual(iid, IParameterChanges::iid)) {
            *obj = this;
            return kResultOk;
        }
        *obj = nullptr;
        return kNoInterface;
    }
    uint32 PLUGIN_API addRef() override { return 1; }
    uint32 PLUGIN_API release() override { return 1; }
    void clear() { used = 0; }
};

// Plain values in the plug-in's units, and their VST3 normalized equivalents.
struct Settings {
    double anger, color, gain, bypass;
    bool angerRamp;
};

static double normalizedFor(int id, double plain) {
    switch (id) {
        case 0: return plain / 100.0;          // ANGER 0..100
        case 1: return plain / 100.0;          // COLOR 0..100
        case 2: return (plain + 36.0) / 72.0;  // GAIN -36..36
        default: return plain >= 0.5 ? 1.0 : 0.0;
    }
}

// ----- VST3 path ------------------------------------------------------------

typedef IPluginFactory* (*GetFactoryFunc)();

static IPluginFactory* loadFactory() {
#if defined(__APPLE__)
    typedef bool (*BundleEntryFunc)(CFBundleRef);
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*)kVst3Path.c_str(), (CFIndex)kVst3Path.size(), true);
    CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, url);
    CFRelease(url);
    if (!bundle) { std::fprintf(stderr, "VST3: bundle not found\n"); return nullptr; }
    if (!CFBundleLoadExecutable(bundle)) { std::fprintf(stderr, "VST3: load failed\n"); return nullptr; }
    auto entry = (BundleEntryFunc)CFBundleGetFunctionPointerForName(bundle, CFSTR("bundleEntry"));
    auto factoryProc = (GetFactoryFunc)CFBundleGetFunctionPointerForName(bundle, CFSTR("GetPluginFactory"));
    if (!entry || !factoryProc || !entry(bundle)) { std::fprintf(stderr, "VST3: entry points missing\n"); return nullptr; }
    return factoryProc();
#else
    // Windows bundle layout: <name>.vst3/Contents/x86_64-win/<name>.vst3
    std::string stem = kVst3Path;
    while (!stem.empty() && (stem.back() == '\\' || stem.back() == '/')) stem.pop_back();
    const size_t slash = stem.find_last_of("\\/");
    const std::string leaf = slash == std::string::npos ? stem : stem.substr(slash + 1);
    const std::string dll = stem + "\\Contents\\x86_64-win\\" + leaf;
    HMODULE mod = LoadLibraryA(dll.c_str());
    if (!mod) { std::fprintf(stderr, "VST3: LoadLibrary failed for %s\n", dll.c_str()); return nullptr; }
    typedef bool (*InitFunc)();
    if (auto init = (InitFunc)GetProcAddress(mod, "InitDll")) init();
    auto factoryProc = (GetFactoryFunc)GetProcAddress(mod, "GetPluginFactory");
    if (!factoryProc) { std::fprintf(stderr, "VST3: GetPluginFactory missing\n"); return nullptr; }
    return factoryProc();
#endif
}

static int renderVst3(const Wav& in, Wav& out, const Settings& s) {
    IPluginFactory* factory = loadFactory();
    if (!factory) return 2;

    TUID cid{};
    bool found = false;
    for (int32 i = 0; i < factory->countClasses(); ++i) {
        PClassInfo info{};
        if (factory->getClassInfo(i, &info) == kResultOk && std::strcmp(info.category, "Audio Module Class") == 0) {
            std::memcpy(cid, info.cid, sizeof cid);
            found = true;
            break;
        }
    }
    if (!found) return 2;

    const TUID& compIid = IComponent::iid;
    const TUID& procIid = IAudioProcessor::iid;
    IComponent* comp = nullptr;
    if (factory->createInstance(reinterpret_cast<FIDString>(cid), reinterpret_cast<FIDString>(&compIid), (void**)&comp) != kResultOk || !comp) return 2;
    HostApp host;
    comp->initialize(&host);
    IAudioProcessor* proc = nullptr;
    comp->queryInterface(procIid, (void**)&proc);
    if (!proc) return 2;

    SpeakerArrangement stereo = SpeakerArr::kStereo;
    proc->setBusArrangements(&stereo, 1, &stereo, 1);
    comp->activateBus(kAudio, kInput, 0, true);
    comp->activateBus(kAudio, kOutput, 0, true);

    ProcessSetup setup{};
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.maxSamplesPerBlock = kBlock;
    setup.sampleRate = in.sampleRate;
    if (proc->setupProcessing(setup) != kResultOk) { std::fprintf(stderr, "VST3: setupProcessing failed\n"); return 3; }
    comp->setActive(true);
    proc->setProcessing(true);

    const size_t total = in.ch[0].size();
    out.ch[0].assign(total, 0.f);
    out.ch[1].assign(total, 0.f);
    out.sampleRate = in.sampleRate;

    ParamChanges changes;
    bool first = true;
    for (size_t pos = 0; pos < total; pos += kBlock) {
        const int n = (int)std::min<size_t>(kBlock, total - pos);
        changes.clear();
        double anger = s.anger;
        if (s.angerRamp) anger = 100.0 * double(pos) / double(total);
        if (first || s.angerRamp) {
            int32 idx = 0;
            ParamQueue* q = (ParamQueue*)changes.addParameterData(0, idx);
            if (q) q->addPoint(0, normalizedFor(0, anger), idx);
            if (first) {
                ParamQueue* c = (ParamQueue*)changes.addParameterData(1, idx);
                if (c) c->addPoint(0, normalizedFor(1, s.color), idx);
                ParamQueue* g = (ParamQueue*)changes.addParameterData(2, idx);
                if (g) g->addPoint(0, normalizedFor(2, s.gain), idx);
                ParamQueue* b = (ParamQueue*)changes.addParameterData(3, idx);
                if (b) b->addPoint(0, normalizedFor(3, s.bypass), idx);
            }
        }

        float* inPtr[2] = {const_cast<float*>(in.ch[0].data() + pos), const_cast<float*>(in.ch[1].data() + pos)};
        float* outPtr[2] = {out.ch[0].data() + pos, out.ch[1].data() + pos};
        AudioBusBuffers inBus{};
        inBus.numChannels = 2;
        inBus.channelBuffers32 = inPtr;
        AudioBusBuffers outBus{};
        outBus.numChannels = 2;
        outBus.channelBuffers32 = outPtr;

        ProcessData data{};
        data.processMode = kRealtime;
        data.symbolicSampleSize = kSample32;
        data.numSamples = n;
        data.numInputs = 1;
        data.numOutputs = 1;
        data.inputs = &inBus;
        data.outputs = &outBus;
        data.inputParameterChanges = &changes;
        if (proc->process(data) != kResultOk) { std::fprintf(stderr, "VST3: process failed at %zu\n", pos); return 4; }
        first = false;
    }

    proc->setProcessing(false);
    comp->setActive(false);
    comp->terminate();
    proc->release();
    comp->release();
    factory->release();
    return 0;
}

#if defined(__APPLE__)
// ----- AU path --------------------------------------------------------------

struct AuInput {
    const Wav* wav;
    size_t pos;
};

static OSStatus auInputProc(void* refCon, AudioUnitRenderActionFlags*, const AudioTimeStamp*, UInt32, UInt32 frames, AudioBufferList* io) {
    AuInput* src = (AuInput*)refCon;
    for (UInt32 c = 0; c < io->mNumberBuffers && c < 2; ++c) {
        float* dst = (float*)io->mBuffers[c].mData;
        const std::vector<float>& v = src->wav->ch[c];
        for (UInt32 i = 0; i < frames; ++i) dst[i] = src->pos + i < v.size() ? v[src->pos + i] : 0.f;
        io->mBuffers[c].mDataByteSize = frames * sizeof(float);
    }
    return noErr;
}

static int renderAu(const Wav& in, Wav& out, const Settings& s) {
    AudioComponentDescription desc{};
    desc.componentType = kAudioUnitType_Effect;
    desc.componentSubType = 'APm1';
    desc.componentManufacturer = 'APLb';
    AudioComponent comp = AudioComponentFindNext(nullptr, &desc);
    if (!comp) { std::fprintf(stderr, "AU: component not found\n"); return 2; }
    AudioUnit unit = nullptr;
    if (AudioComponentInstanceNew(comp, &unit) != noErr || !unit) return 2;

    UInt32 maxFrames = kBlock;
    AudioUnitSetProperty(unit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maxFrames, sizeof maxFrames);
    AudioStreamBasicDescription asbd{};
    asbd.mSampleRate = in.sampleRate;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
    asbd.mBytesPerPacket = 4;
    asbd.mFramesPerPacket = 1;
    asbd.mBytesPerFrame = 4;
    asbd.mChannelsPerFrame = 2;
    asbd.mBitsPerChannel = 32;
    AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, sizeof asbd);
    AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &asbd, sizeof asbd);

    AuInput src{&in, 0};
    AURenderCallbackStruct cb{auInputProc, &src};
    AudioUnitSetProperty(unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &cb, sizeof cb);
    if (AudioUnitInitialize(unit) != noErr) { std::fprintf(stderr, "AU: initialize failed\n"); return 3; }

    AudioUnitSetParameter(unit, 0, kAudioUnitScope_Global, 0, (AudioUnitParameterValue)s.anger, 0);
    AudioUnitSetParameter(unit, 1, kAudioUnitScope_Global, 0, (AudioUnitParameterValue)s.color, 0);
    AudioUnitSetParameter(unit, 2, kAudioUnitScope_Global, 0, (AudioUnitParameterValue)s.gain, 0);
    AudioUnitSetParameter(unit, 3, kAudioUnitScope_Global, 0, (AudioUnitParameterValue)s.bypass, 0);

    const size_t total = in.ch[0].size();
    out.ch[0].assign(total, 0.f);
    out.ch[1].assign(total, 0.f);
    out.sampleRate = in.sampleRate;

    for (size_t pos = 0; pos < total; pos += kBlock) {
        const UInt32 n = (UInt32)std::min<size_t>(kBlock, total - pos);
        if (s.angerRamp) {
            double anger = 100.0 * double(pos) / double(total);
            AudioUnitSetParameter(unit, 0, kAudioUnitScope_Global, 0, (AudioUnitParameterValue)anger, 0);
        }
        src.pos = pos;
        float* outPtr[2] = {out.ch[0].data() + pos, out.ch[1].data() + pos};
        AudioBufferList* abl = (AudioBufferList*)std::malloc(offsetof(AudioBufferList, mBuffers) + 2 * sizeof(AudioBuffer));
        abl->mNumberBuffers = 2;
        for (int c = 0; c < 2; ++c) {
            abl->mBuffers[c].mNumberChannels = 1;
            abl->mBuffers[c].mDataByteSize = n * sizeof(float);
            abl->mBuffers[c].mData = outPtr[c];
        }
        AudioTimeStamp ts{};
        ts.mSampleTime = (Float64)pos;
        ts.mFlags = kAudioTimeStampSampleTimeValid;
        AudioUnitRenderActionFlags flags = 0;
        OSStatus rc = AudioUnitRender(unit, &flags, &ts, 0, n, abl);
        std::free(abl);
        if (rc != noErr) { std::fprintf(stderr, "AU: render failed %d at %zu\n", (int)rc, pos); return 4; }
    }

    AudioUnitUninitialize(unit);
    AudioComponentInstanceDispose(unit);
    return 0;
}
#endif

int main(int argc, char** argv) {
    if (argc != 10) {
        std::fprintf(stderr, "usage: %s <vst3|au> <plugin> <in.wav> <out.wav> <anger> <color> <gain> <bypass> <none|anger-ramp>\n", argv[0]);
        return 1;
    }
    if (const char* b = std::getenv("AP_PARITY_BLOCK")) kBlock = std::max(1, std::atoi(b));
    kVst3Path = argv[2];
    Settings s{std::atof(argv[5]), std::atof(argv[6]), std::atof(argv[7]), std::atof(argv[8]), std::strcmp(argv[9], "anger-ramp") == 0};
    Wav in, out;
    if (!readWav(argv[3], in)) { std::fprintf(stderr, "cannot read %s\n", argv[3]); return 1; }
#if defined(__APPLE__)
    int rc = std::strcmp(argv[1], "au") == 0 ? renderAu(in, out, s) : renderVst3(in, out, s);
#else
    if (std::strcmp(argv[1], "au") == 0) { std::fprintf(stderr, "AU is macOS only\n"); return 1; }
    int rc = renderVst3(in, out, s);
#endif
    if (rc != 0) return rc;
    if (!writeWav(argv[4], out.ch, in.sampleRate)) { std::fprintf(stderr, "cannot write %s\n", argv[4]); return 1; }
    std::printf("rendered %s frames=%zu rate=%.0f block=%d anger=%g color=%g gain=%g bypass=%g ramp=%s\n", argv[1], out.ch[0].size(), in.sampleRate, kBlock, s.anger, s.color, s.gain, s.bypass, s.angerRamp ? "anger" : "none");
    return 0;
}
