/*
  ==============================================================================
    Source/MainComponent.h
  ==============================================================================
*/
#pragma once
#include "Core/PluginCanvas.h"
#include "Core/PluginData.h"
#include "Core/PluginGenerator.h"
#include "Core/PropertiesPanel.h"
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

// Componente principal de la aplicacion y modelo de la barra de menus.
class MainComponent : public juce::Component, public juce::MenuBarModel {
public:
  MainComponent();
  ~MainComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

  // Implementacion requerida por MenuBarModel.
  juce::StringArray getMenuBarNames() override;
  juce::PopupMenu getMenuForIndex(int topLevelMenuIndex,
                                  const juce::String &menuName) override;
  void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

private:
  void syncPresetDspCode();
  void syncLedTogglesFromProject();
  void normalizeSelectorsForCurrentEffect();
  bool hasUnsavedProjectChanges();
  void promptSaveBeforeNewProject();
  void resetProject();
  void saveProjectAs(std::function<void(bool)> onComplete = {});
  void verifyBuildEnvironment();

  // Barra de menus visible en la parte superior.
  juce::MenuBarComponent menuBar;

  // Controles principales de la interfaz.
  juce::Label toolsLabel{{}, "Herramientas"};
  juce::Label listLabel{{}, juce::String::fromUTF8("Lienzo de Diseño")};

  juce::TextButton addSliderBtn{"+ Slider"};
  juce::TextButton addToggleBtn{"+ Switch"};
  juce::TextButton addKnobBtn{"+ Knob"};
  juce::TextButton addSelectorBtn{"+ Selector"};
  juce::TextButton deleteBtn{"Eliminar"};
  juce::TextButton clearBtn{"Borrar Todo"};
  juce::TextButton generateBtn{"GENERAR LV2"};

  juce::Label ledToolsLabel{{}, "Indicadores"};
  juce::ToggleButton inputLedToggle{"Input LED"};
  juce::ToggleButton outputLedToggle{"Output LED"};
  juce::ToggleButton clipLedToggle{"Clip LED"};
  juce::ToggleButton levelMeterToggle{"Level Meter"};
  juce::ToggleButton rmsMeterToggle{"RMS Meter"};
  juce::ToggleButton processingLedToggle{"Processing LED"};

  PluginCanvas canvas;
  PropertiesPanel propertiesPanel;
  PluginData::Project project;
  PluginGenerator generator;

  std::unique_ptr<juce::FileChooser> fileChooser;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
