#pragma once
#include <string>

namespace Templates {
// ==============================================================================
// 1. DSP HEADER
// ==============================================================================
const std::string processorHeader = R"jv(
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

class DspEngine
{
public:
    DspEngine();
    ~DspEngine();

    void prepare(double sampleRate, int samplesPerBlock);
    
    void process(float* const* inputChannelData, float* const* outputChannelData, 
                 int numInChannels, int numOutChannels, int numSamples, 
                 const std::vector<float>& params);

    double getSampleRate() const { return currentSampleRate; }

private:
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    //VARIABLES USUARIO
    {{USER_VARS}}
};
)jv";

// ==============================================================================
// 2. DSP CPP
// ==============================================================================
const std::string processorCpp = R"jv(
#include "PluginProcessor.h"
#include <cmath> 

DspEngine::DspEngine() {}
DspEngine::~DspEngine() {}

void DspEngine::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    //INIT USUARIO
    {{INIT_CODE}}
}

void DspEngine::process(float* const* inputChannelData, float* const* outputChannelData, 
                        int numInChannels, int numOutChannels, int numSamples, 
                        const std::vector<float>& params)
{
    juce::AudioBuffer<float> buffer(outputChannelData, numOutChannels, numSamples);

    int minChannels = std::min(numInChannels, numOutChannels);
    for (int i = 0; i < minChannels; ++i)
        buffer.copyFrom(i, 0, inputChannelData[i], numSamples);
        
    if (numOutChannels > numInChannels && numInChannels > 0)
        for (int i = numInChannels; i < numOutChannels; ++i)
            buffer.copyFrom(i, 0, inputChannelData[0], numSamples);

    int totalNumInputChannels = numOutChannels;

    // ==========================================================================
    // AYUDA PARA SELECTORES (IMPORTANTE)
    // ==========================================================================
    // Los parámetros discretos (Selector) se usan así:
    //
    // int mode = (int)params[X];
    //
    // Ejemplo típico:
    //
    // int mode = (int)params[0];
    //
    // switch (mode)
    // {
    //     case 0: // Soft clip
    //         break;
    //     case 1: // Hard clip
    //         break;
    //     case 2: // Foldback
    //         break;
    // }
    //
    // IMPORTANTE:
    // - Los valores son enteros (0,1,2...)
    // - El rango viene definido por numSteps en el IDE
    // ==========================================================================

    //DSP USUARIO
    {{DSP_CODE}}
}
)jv";

// ==============================================================================
// 3. DUMMY HEADER
// ==============================================================================
const std::string editorHeader = R"jv(
#pragma once
)jv";

// ==============================================================================
// 4. LV2 WRAPPER (SIN GUI)
// ==============================================================================
const std::string editorCpp = R"jv(
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cmath>
#include <lv2/core/lv2.h>
#include "PluginProcessor.h"

#define NUM_INPUTS {{NUM_INPUTS}}
#define NUM_OUTPUTS {{NUM_OUTPUTS}}
#define PARAM_START_INDEX (NUM_INPUTS + NUM_OUTPUTS)
#define NUM_PARAMS {{NUM_PARAMS}} 
#define NUM_MONITOR_PORTS {{NUM_MONITOR_PORTS}}
#define MONITOR_START_INDEX (PARAM_START_INDEX + NUM_PARAMS)

#ifndef PLUGIN_URI_STRING
#define PLUGIN_URI_STRING "http://upm.es/plugins/MiEfectoTFG"
#endif

struct Lv2Plugin {
    std::vector<float*> inputChans;
    std::vector<float*> outputChans;
    std::vector<float*> paramPtrs;
    std::vector<float*> monitorPtrs;
    DspEngine* dsp;
};

static LV2_Handle instantiate(const LV2_Descriptor*, double rate, const char*, const LV2_Feature* const*) {
    Lv2Plugin* plugin = new Lv2Plugin();
    plugin->dsp = new DspEngine();
    plugin->dsp->prepare(rate, 512);

    plugin->inputChans.resize(NUM_INPUTS, nullptr);
    plugin->outputChans.resize(NUM_OUTPUTS, nullptr);
    plugin->paramPtrs.resize(NUM_PARAMS, nullptr);
    plugin->monitorPtrs.resize(NUM_MONITOR_PORTS, nullptr);

    return (LV2_Handle)plugin;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
    Lv2Plugin* plugin = (Lv2Plugin*)instance;

    if (port < NUM_INPUTS)
        plugin->inputChans[port] = (float*)data;
    else if (port < NUM_INPUTS + NUM_OUTPUTS)
        plugin->outputChans[port - NUM_INPUTS] = (float*)data;
    else if (port < MONITOR_START_INDEX) {
        int paramIndex = (int)port - PARAM_START_INDEX;
        if (paramIndex >= 0 && paramIndex < NUM_PARAMS)
            plugin->paramPtrs[paramIndex] = (float*)data;
    } else {
        int monitorIndex = (int)port - MONITOR_START_INDEX;
        if (monitorIndex >= 0 && monitorIndex < NUM_MONITOR_PORTS)
            plugin->monitorPtrs[monitorIndex] = (float*)data;
    }
}

static void run(LV2_Handle instance, uint32_t n_samples) {
    Lv2Plugin* plugin = (Lv2Plugin*)instance;

    // 🔥 VECTOR LIMPIO Y ESTABLE
    std::vector<float> currentValues(NUM_PARAMS);

    for (int i = 0; i < NUM_PARAMS; ++i)
        currentValues[i] = plugin->paramPtrs[i] ? *(plugin->paramPtrs[i]) : 0.0f;

    plugin->dsp->process(plugin->inputChans.data(), plugin->outputChans.data(),
                         NUM_INPUTS, NUM_OUTPUTS, (int)n_samples, currentValues);

    // 🔥 SIGNAL INDICATORS
    if (NUM_MONITOR_PORTS > 0)
    {
        float peak = 0.0f;

        for (int ch = 0; ch < NUM_OUTPUTS; ++ch)
        {
            float* data = plugin->outputChans[ch];
            if (!data)
                continue;

            for (uint32_t i = 0; i < n_samples; ++i)
                peak = std::max(peak, std::abs(data[i]));
        }

        float signalLed = peak > 0.01f ? 1.0f : 0.0f;
        float clipLed = peak > 0.95f ? 1.0f : 0.0f;
        float levelMeter = juce::jlimit(0.0f, 1.0f, peak);

        if (NUM_MONITOR_PORTS > 0 && plugin->monitorPtrs[0])
            *(plugin->monitorPtrs[0]) = signalLed;
        if (NUM_MONITOR_PORTS > 1 && plugin->monitorPtrs[1])
            *(plugin->monitorPtrs[1]) = clipLed;
        if (NUM_MONITOR_PORTS > 2 && plugin->monitorPtrs[2])
            *(plugin->monitorPtrs[2]) = levelMeter;
    }
}

static void cleanup(LV2_Handle instance) {
    Lv2Plugin* plugin = (Lv2Plugin*)instance;
    delete plugin->dsp;
    delete plugin;
}

static void activate(LV2_Handle) {}
static void deactivate(LV2_Handle) {}
static const void* extension_data(const char*) { return NULL; }

static const LV2_Descriptor descriptor = {
    PLUGIN_URI_STRING,
    instantiate, connect_port, activate, run, deactivate, cleanup, extension_data
};

extern "C" {
    LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
        return index == 0 ? &descriptor : NULL;
    }
}
)jv";

// ==============================================================================
// 5. CMAKE
// ==============================================================================
const std::string cmakeFile = R"jv(
cmake_minimum_required(VERSION 3.15)
project(SimpleLv2Plugin)

set(JUCE_USE_CURL OFF CACHE BOOL "Disable CURL" FORCE)
set(JUCE_WEB_BROWSER OFF CACHE BOOL "Disable Web Browser" FORCE)
add_compile_definitions(JUCE_USE_CURL=0)
add_compile_definitions(JUCE_WEB_BROWSER=0)

add_subdirectory(JUCE)

add_library(DspLib STATIC Source/PluginProcessor.cpp)
target_link_libraries(DspLib PRIVATE juce::juce_core juce::juce_audio_basics)

add_library(MiEfectoDSP MODULE Source/PluginEditor.cpp)
set_target_properties(MiEfectoDSP PROPERTIES PREFIX "")
set_target_properties(MiEfectoDSP PROPERTIES SUFFIX ".so")
# EXTRA BUILD CONFIG
{{EXTRA_INCLUDE_DIRS}}
target_link_libraries(MiEfectoDSP PRIVATE DspLib juce::juce_core juce::juce_audio_basics {{EXTRA_LIBRARIES}})

target_compile_definitions(MiEfectoDSP PRIVATE PLUGIN_URI_STRING="{{PLUGIN_URI}}")

file(WRITE ${CMAKE_BINARY_DIR}/manifest.ttl 
"@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .
<{{PLUGIN_URI}}> a lv2:Plugin ; lv2:binary <MiEfectoDSP.so> ; rdfs:seeAlso <plugin.ttl> .")

file(WRITE ${CMAKE_BINARY_DIR}/plugin.ttl 
"@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .
@prefix doap: <http://usefulinc.com/ns/doap#> .
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .
@prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
<{{PLUGIN_URI}}> a lv2:Plugin, lv2:AudioPlugin ;
    doap:name \"{{PLUGIN_NAME}}\" ;
    lv2:port {{AUDIO_PORTS_TTL}}
{{TTL_PORTS}}
{{MONITOR_TTL_PORTS}}
    .")

add_custom_command(TARGET MiEfectoDSP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/manifest.ttl $<TARGET_FILE_DIR:MiEfectoDSP>
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/plugin.ttl $<TARGET_FILE_DIR:MiEfectoDSP>
)
)jv";
} // namespace Templates
