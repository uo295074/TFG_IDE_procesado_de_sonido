#pragma once
#include <juce_core/juce_core.h>
#include "PluginData.h"

class PluginGenerator
{
public:
    PluginGenerator();
    
    // La función principal que llamará el botón
    void createPluginFiles(PluginData::Project& project);

private:
    juce::File desktopDir;
};