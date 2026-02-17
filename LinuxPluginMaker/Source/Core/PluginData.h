/*
  ==============================================================================
    Source/Core/PluginData.h
  ==============================================================================
*/
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace PluginData
{
    enum class ComponentType { Slider, Toggle };

    struct Component
    {
        int index;
        ComponentType type;
        juce::String name;
        juce::String symbol;
        float min = 0.0f;
        float max = 1.0f;
        float def = 0.5f;
    };

    struct Project
    {
        // --- NUEVOS DATOS GLOBALES (RU-01) ---
        juce::String pluginName = "Mi Plugin Nuevo";
        juce::String manufacturer = "Mi Nombre";
        juce::String pluginURI = "http://miweb.com/plugins/miplugin";
        
        // La lista de componentes que ya tenías
        std::vector<Component> components;

        // Métodos auxiliares para gestión rápida
        void addSlider(const juce::String& n, float mn, float mx, float df) {
            Component c; c.type = ComponentType::Slider; c.name = n; 
            c.min = mn; c.max = mx; c.def = df; c.index = components.size();
            components.push_back(c);
        }
        
        void addToggle(const juce::String& n) {
            Component c; c.type = ComponentType::Toggle; c.name = n; 
            c.index = components.size();
            components.push_back(c);
        }
    };
}