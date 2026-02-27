/*
  ==============================================================================
    Source/MainComponent.h
  ==============================================================================
*/
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "Core/PluginData.h"
#include "Core/PluginGenerator.h"
#include "Core/PluginCanvas.h"
#include "Core/PropertiesPanel.h"

// 1. Heredamos de MenuBarModel
class MainComponent : public juce::Component,
                      public juce::MenuBarModel 
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // 2. Métodos OBLIGATORIOS del Menú
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

private:
    // 3. El componente visual de la barra
    juce::MenuBarComponent menuBar;

    // (El resto sigue igual)
    juce::Label toolsLabel { {}, "Herramientas" };
    juce::Label listLabel { {}, "Lienzo de Diseño" };

    juce::TextButton addSliderBtn { "+ Slider" };
    juce::TextButton addToggleBtn { "+ Switch" };
    juce::TextButton addKnobBtn{ "+ Knob" };
    juce::TextButton clearBtn { "Borrar Todo" };
    juce::TextButton generateBtn { "GENERAR LV2" };

    PluginCanvas canvas;
    PropertiesPanel propertiesPanel;
    PluginData::Project project;
    PluginGenerator generator;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};