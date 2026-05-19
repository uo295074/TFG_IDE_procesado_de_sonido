/*
  ==============================================================================
    Source/Core/PluginGenerator.h
  ==============================================================================
*/
#pragma once
#include "PluginData.h"
#include <juce_core/juce_core.h>


class PluginGenerator {
public:
  PluginGenerator();

  static juce::String getBuiltinDspCode(PluginData::AlgorithmType algorithm);
  static juce::String getSafePluginFileName(const PluginData::Project &project);
  juce::File getGeneratedProjectDir(const PluginData::Project &project) const;

  void createPluginFiles(PluginData::Project &project);

  // --- NUEVO MÉTODO: Compila e instala automáticamente ---
  juce::String compileAndInstallPlugin(const PluginData::Project &project);
  bool validateProject(const PluginData::Project &project, juce::String &error);

private:
  juce::File desktopDir;
};
