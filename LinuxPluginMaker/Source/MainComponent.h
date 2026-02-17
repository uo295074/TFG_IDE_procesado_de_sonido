/*
  ==============================================================================
    Source/MainComponent.h
  ==============================================================================
*/

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "Core/PluginData.h"      // <--- Importante
#include "Core/PluginGenerator.h" // <--- Tu generador

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // --- NUESTRO MODELO DE DATOS ---
    PluginData::Project project;
    PluginGenerator generator;

    // --- ZONA IZQUIERDA (HERRAMIENTAS) ---
    juce::Label toolsLabel { {}, "Herramientas" };
    juce::TextButton addSliderBtn { "+ Añadir Slider" };
    juce::TextButton addToggleBtn { "+ Añadir Switch" };
    juce::TextButton clearBtn     { "Borrar Todo" };

    // --- ZONA CENTRAL (VISUALIZACIÓN) ---
    juce::Label listLabel { {}, "Componentes del Plugin" };
    // Usamos un TextEditor simple para mostrar la lista por ahora
    juce::TextEditor componentsLog; 

    // --- BOTÓN PRINCIPAL ---
    juce::TextButton generateBtn { "GENERAR PROYECTO LV2" };

    // Función auxiliar para actualizar la lista visual
    void updateListView();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};