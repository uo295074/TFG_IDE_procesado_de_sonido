/*
  ==============================================================================
    Source/Core/PluginGenerator.h
  ==============================================================================
*/
#pragma once
#include <juce_core/juce_core.h>
#include "PluginData.h"

class PluginGenerator
{
public:
    PluginGenerator();

    void createPluginFiles(PluginData::Project& project);
    
    // --- NUEVO MÉTODO: Compila e instala automáticamente ---
    juce::String compileAndInstallPlugin(const PluginData::Project& project);

private:
    juce::File desktopDir;
};