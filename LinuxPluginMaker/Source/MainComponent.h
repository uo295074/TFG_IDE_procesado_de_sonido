/*
  ==============================================================================
    Ruta: Source/MainComponent.h
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Core/PluginGenerator.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // UI Elements
    juce::Label nameLabel;
    juce::TextEditor nameEditor;
    
    juce::Label codeLabel;
    juce::TextEditor codeEditor; // <--- NUEVO: Caja grande para código
    
    juce::TextButton generateButton;

    // Logic
    PluginGenerator generator;
    void generatePlugin();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
