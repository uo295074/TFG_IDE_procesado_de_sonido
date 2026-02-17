/*
  ==============================================================================
    Source/Core/PluginData.h
    Definición de los componentes (Sliders, Toggles, etc.)
  ==============================================================================
*/

#pragma once
#include <juce_core/juce_core.h>
#include <vector>

namespace PluginData
{
    enum class ComponentType { Slider, Toggle };

    struct Component
    {
        ComponentType type;
        juce::String name;
        juce::String symbol;
        float min = 0.0f;
        float max = 1.0f;
        float def = 0.0f;
        int index = -1; // Puerto LV2
    };

    struct Project
    {
        juce::String pluginName = "Mi Efecto TFG";
        std::vector<Component> components;

        // Añadir Slider
        void addSlider(juce::String name, float min, float max, float def)
        {
            Component c;
            c.type = ComponentType::Slider;
            c.name = name;
            c.symbol = cleanSymbol(name);
            c.min = min; c.max = max; c.def = def;
            c.index = 4 + (int)components.size(); 
            components.push_back(c);
        }

        // Añadir Toggle (Interruptor)
        void addToggle(juce::String name)
        {
            Component c;
            c.type = ComponentType::Toggle;
            c.name = name;
            c.symbol = cleanSymbol(name);
            c.min = 0.0f; c.max = 1.0f; c.def = 0.0f;
            c.index = 4 + (int)components.size();
            components.push_back(c);
        }

        // Limpia el nombre para usarlo en código (Ej: "Volumen Master" -> "volumen_master")
        juce::String cleanSymbol(juce::String input)
        {
            return input.toLowerCase().retainCharacters("abcdefghijklmnopqrstuvwxyz0123456789_");
        }
    };
}