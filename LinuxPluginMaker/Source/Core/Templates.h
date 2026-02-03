/*
  ==============================================================================
    Source/Core/Templates.h
    (Versión FINAL: Con extern "C" para que Reaper entienda el nombre)
  ==============================================================================
*/

#pragma once
#include <string>

namespace Templates
{
    // 1. DSP HEADER (Igual que antes)
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
    void process(float* const* inputChannelData, float* const* outputChannelData, int numChannels, int numSamples);
private:
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};
)jv";

    // 2. DSP CPP (Igual que antes)
    const std::string processorCpp = R"jv(
#include "PluginProcessor.h"

DspEngine::DspEngine() {}
DspEngine::~DspEngine() {}

void DspEngine::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
}

void DspEngine::process(float* const* inputChannelData, float* const* outputChannelData, int numChannels, int numSamples)
{
    juce::AudioBuffer<float> buffer(outputChannelData, numChannels, numSamples);

    if (inputChannelData != outputChannelData)
    {
        for (int i = 0; i < numChannels; ++i)
            buffer.copyFrom(i, 0, inputChannelData[i], numSamples);
    }

    auto totalNumInputChannels = numChannels;
    auto totalNumOutputChannels = numChannels;

    // *** USER_CODE_TAG ***
}
)jv";

    // 3. DUMMY HEADER (Igual que antes)
    const std::string editorHeader = R"jv(
#pragma once
)jv";

    // 4. LV2 WRAPPER (AQUÍ ESTÁ EL CAMBIO CRÍTICO: extern "C")
    const std::string editorCpp = R"jv(
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <lv2/core/lv2.h>
#include "PluginProcessor.h"

#define PORT_AUDIO_IN_L  0
#define PORT_AUDIO_IN_R  1
#define PORT_AUDIO_OUT_L 2
#define PORT_AUDIO_OUT_R 3

struct Lv2Plugin {
    float* inputL; float* inputR;
    float* outputL; float* outputR;
    DspEngine* dsp;
    std::vector<float*> inputChans;
    std::vector<float*> outputChans;
};

static LV2_Handle instantiate(const LV2_Descriptor*, double rate, const char*, const LV2_Feature* const*) {
    Lv2Plugin* plugin = new Lv2Plugin();
    plugin->dsp = new DspEngine();
    plugin->dsp->prepare(rate, 512);
    plugin->inputChans.resize(2);
    plugin->outputChans.resize(2);
    return (LV2_Handle)plugin;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
    Lv2Plugin* plugin = (Lv2Plugin*)instance;
    switch (port) {
        case PORT_AUDIO_IN_L:  plugin->inputL = (float*)data; break;
        case PORT_AUDIO_IN_R:  plugin->inputR = (float*)data; break;
        case PORT_AUDIO_OUT_L: plugin->outputL = (float*)data; break;
        case PORT_AUDIO_OUT_R: plugin->outputR = (float*)data; break;
    }
}

static void run(LV2_Handle instance, uint32_t n_samples) {
    Lv2Plugin* plugin = (Lv2Plugin*)instance;
    plugin->inputChans[0] = plugin->inputL;
    plugin->inputChans[1] = plugin->inputR;
    plugin->outputChans[0] = plugin->outputL;
    plugin->outputChans[1] = plugin->outputR;

    plugin->dsp->process(plugin->inputChans.data(), plugin->outputChans.data(), 2, (int)n_samples);
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
    "http://upm.es/plugins/MiEfectoTFG",
    instantiate, connect_port, activate, run, deactivate, cleanup, extension_data
};

// --- EL FIX: extern "C" PARA QUE REAPER LO ENCUENTRE ---
extern "C" {
    LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
        return index == 0 ? &descriptor : NULL;
    }
}
)jv";

    // 5. CMAKE (Igual que la última vez que funcionó)
    const std::string cmakeFile = R"jv(
cmake_minimum_required(VERSION 3.15)
project(SimpleLv2Plugin)

# --- LLAVE 1: Configuración de CMake ---
set(JUCE_USE_CURL OFF CACHE BOOL "Disable CURL" FORCE)
set(JUCE_WEB_BROWSER OFF CACHE BOOL "Disable Web Browser" FORCE)

# --- LLAVE 2: Inyección en el C++ (OBLIGATORIO) ---
# Esto asegura que el código fuente vea el "0" y no compile las llamadas a curl
add_compile_definitions(JUCE_USE_CURL=0)
add_compile_definitions(JUCE_WEB_BROWSER=0)

# Añadimos JUCE
add_subdirectory(/media/sf_TFG_COMPARTIDO/JUCE subprojects/juce)

# Librería DSP
add_library(DspLib STATIC Source/PluginProcessor.cpp)
target_link_libraries(DspLib PRIVATE juce::juce_core juce::juce_audio_basics)

# Plugin LV2
add_library(MiEfectoDSP MODULE Source/PluginEditor.cpp)
set_target_properties(MiEfectoDSP PROPERTIES PREFIX "")
set_target_properties(MiEfectoDSP PROPERTIES SUFFIX ".so")

target_link_libraries(MiEfectoDSP PRIVATE 
    DspLib 
    juce::juce_core 
    juce::juce_audio_basics
)

# Archivos TTL
file(WRITE ${CMAKE_BINARY_DIR}/manifest.ttl 
"@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .
<http://upm.es/plugins/MiEfectoTFG> a lv2:Plugin ; lv2:binary <MiEfectoDSP.so> ; rdfs:seeAlso <plugin.ttl> .")

file(WRITE ${CMAKE_BINARY_DIR}/plugin.ttl 
"@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .
@prefix doap: <http://usefulinc.com/ns/doap#> .
<http://upm.es/plugins/MiEfectoTFG> a lv2:Plugin, lv2:AudioPlugin ;
    doap:name \"Mi Efecto TFG (Custom)\" ;
    lv2:port [ a lv2:InputPort, lv2:AudioPort ; lv2:index 0 ; lv2:symbol \"in_l\" ; lv2:name \"In L\" ],
             [ a lv2:InputPort, lv2:AudioPort ; lv2:index 1 ; lv2:symbol \"in_r\" ; lv2:name \"In R\" ],
             [ a lv2:OutputPort, lv2:AudioPort ; lv2:index 2 ; lv2:symbol \"out_l\" ; lv2:name \"Out L\" ],
             [ a lv2:OutputPort, lv2:AudioPort ; lv2:index 3 ; lv2:symbol \"out_r\" ; lv2:name \"Out R\" ] .")

add_custom_command(TARGET MiEfectoDSP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/manifest.ttl $<TARGET_FILE_DIR:MiEfectoDSP>
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/plugin.ttl $<TARGET_FILE_DIR:MiEfectoDSP>
)
)jv";
}