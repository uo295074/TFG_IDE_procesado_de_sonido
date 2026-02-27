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
    enum class ComponentType { Slider, Toggle, Knob };

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
        
        AlgorithmType currentAlgorithm = AlgorithmType::Gain; 
        std::vector<Component> components;

        void addSlider(const juce::String& n, float mn, float mx, float df) {
            Component c; c.type = ComponentType::Slider; c.name = n; 
            c.min = mn; c.max = mx; c.def = df; c.index = components.size();
            components.push_back(c);
        }

        void addKnob(const juce::String& n, float mn, float mx, float df) {
            Component c; c.type = ComponentType::Knob; c.name = n; 
            c.min = mn; c.max = mx; c.def = df; c.index = components.size();
            components.push_back(c);
        }

        // --- RU-02: GUARDAR A XML ---
        std::unique_ptr<juce::XmlElement> toXml() const
        {
            auto xml = std::make_unique<juce::XmlElement>("LINUX_PLUGIN_MAKER_PROJECT");
            xml->setAttribute("name", pluginName);
            xml->setAttribute("manufacturer", manufacturer);
            xml->setAttribute("uri", pluginURI);
            xml->setAttribute("algorithm", (int)currentAlgorithm);

            auto compsXml = new juce::XmlElement("COMPONENTS");
            for (const auto& comp : components) {
                auto cXml = new juce::XmlElement("COMPONENT");
                cXml->setAttribute("index", comp.index);
                cXml->setAttribute("type", (int)comp.type);
                cXml->setAttribute("name", comp.name);
                cXml->setAttribute("symbol", comp.symbol);
                cXml->setAttribute("min", comp.min);
                cXml->setAttribute("max", comp.max);
                cXml->setAttribute("def", comp.def);
                compsXml->addChildElement(cXml);
            }
            xml->addChildElement(compsXml); // El padre toma control de la memoria
            return xml;
        }

        // --- RU-02: CARGAR DESDE XML ---
        void fromXml(juce::XmlElement* xml)
        {
            if (xml == nullptr || xml->getTagName() != "LINUX_PLUGIN_MAKER_PROJECT") return;

            pluginName = xml->getStringAttribute("name", "Mi Plugin Nuevo");
            manufacturer = xml->getStringAttribute("manufacturer", "Mi Nombre");
            pluginURI = xml->getStringAttribute("uri", "http://miweb.com/plugins/miplugin");
            currentAlgorithm = (AlgorithmType)xml->getIntAttribute("algorithm", 0);

            components.clear();
            if (auto* compsXml = xml->getChildByName("COMPONENTS")) {
                // Iteramos por cada componente guardado
                for (auto* cXml : compsXml->getChildIterator()) {
                    Component c;
                    c.index = cXml->getIntAttribute("index");
                    c.type = (ComponentType)cXml->getIntAttribute("type");
                    c.name = cXml->getStringAttribute("name");
                    c.symbol = cXml->getStringAttribute("symbol");
                    c.min = cXml->getDoubleAttribute("min", 0.0);
                    c.max = cXml->getDoubleAttribute("max", 1.0);
                    c.def = cXml->getDoubleAttribute("def", 0.5);
                    components.push_back(c);
                }
            }
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