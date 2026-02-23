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
    // 1. DEFINIMOS LOS TIPOS DE EFECTO DISPONIBLES
    enum class AlgorithmType 
    { 
        Gain,       // Multiplicación simple
        Distortion, // Saturación (tanh)
        Tremolo     // Modulación de amplitud (LFO)
    };

    // (Esto sigue igual)
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
        juce::String pluginName = "Mi Plugin Nuevo";
        juce::String manufacturer = "Mi Nombre";
        juce::String pluginURI = "http://miweb.com/plugins/miplugin";
        
        // 2. AÑADIMOS LA VARIABLE DEL ALGORITMO (Por defecto Gain)
        AlgorithmType currentAlgorithm = AlgorithmType::Gain; 
        
        std::vector<Component> components;

        // Ayudante para añadir sliders rápido
        void addSlider(const juce::String& n, float mn, float mx, float df) {
            Component c; c.type = ComponentType::Slider; c.name = n; 
            c.min = mn; c.max = mx; c.def = df; c.index = components.size();
            components.push_back(c);
        }
    };
    
    // Función auxiliar para convertir el enum a texto (útil para la UI)
    static juce::String getAlgorithmName(AlgorithmType type)
    {
        switch (type)
        {
            case AlgorithmType::Gain:       return "Control de Ganancia (Básico)";
            case AlgorithmType::Distortion: return "Distorsión (Saturación)";
            case AlgorithmType::Tremolo:    return "Trémolo (Modulación AM)";
            default: return "Desconocido";
        }
    }
}