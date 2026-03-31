/*
  ==============================================================================
    Source/Core/PluginData.h
    Actualizado con soporte para Código DSP Personalizado
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
        Gain,       
        Distortion, 
        Tremolo,     
        Custom      
    };

    enum class ComponentType { Slider, Toggle, Knob };

    enum class ParamRole
    {
        None,
        Gain,
        Drive,
        Mix,
        Tone,
        Frequency,
        Depth
    };

    struct Component
    {
        int index;
        ComponentType type;
        juce::String name;
        juce::String symbol;
        ParamRole role = ParamRole::None;
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

        int numInputs = 2;  
        int numOutputs = 2; 

        // --- NUEVO: Variable para guardar el código del usuario ---
        // Le ponemos un texto por defecto para guiar al técnico
        juce::String customDspCode = 
            "// --- TU CÓDIGO DSP AQUÍ ---\n"
            "// Usa 'channelData[sample]' para leer/escribir el audio.\n"
            "// Usa 'params[0]', 'params[1]', etc., para leer tus controles.\n\n"
            "for (int channel = 0; channel < totalNumInputChannels; ++channel)\n"
            "{\n"
            "    auto* channelData = buffer.getWritePointer (channel);\n"
            "    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)\n"
            "    {\n"
            "        // Ejemplo: channelData[sample] *= params[0];\n"
            "    }\n"
            "}\n";
        // ----------------------------------------------------------

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

        std::unique_ptr<juce::XmlElement> toXml() const
        {
            auto xml = std::make_unique<juce::XmlElement>("LINUX_PLUGIN_MAKER_PROJECT");
            xml->setAttribute("name", pluginName);
            xml->setAttribute("manufacturer", manufacturer);
            xml->setAttribute("uri", pluginURI);
            xml->setAttribute("algorithm", (int)currentAlgorithm);
            xml->setAttribute("numInputs", numInputs);
            xml->setAttribute("numOutputs", numOutputs);

            // --- NUEVO: Guardar el código C++ en el XML ---
            auto dspXml = new juce::XmlElement("CUSTOM_DSP");
            dspXml->addTextElement(customDspCode);
            xml->addChildElement(dspXml);
            // ----------------------------------------------

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
                cXml->setAttribute("role", (int)comp.role);
                compsXml->addChildElement(cXml);
            }
            xml->addChildElement(compsXml); 
            return xml;
        }

        void fromXml(juce::XmlElement* xml)
        {
            if (xml == nullptr || xml->getTagName() != "LINUX_PLUGIN_MAKER_PROJECT") return;

            pluginName = xml->getStringAttribute("name", "Mi Plugin Nuevo");
            manufacturer = xml->getStringAttribute("manufacturer", "Mi Nombre");
            pluginURI = xml->getStringAttribute("uri", "http://miweb.com/plugins/miplugin");
            currentAlgorithm = (AlgorithmType)xml->getIntAttribute("algorithm", 0);
            numInputs = xml->getIntAttribute("numInputs", 2);
            numOutputs = xml->getIntAttribute("numOutputs", 2);

            // --- NUEVO: Leer el código C++ desde el XML ---
            if (auto* dspXml = xml->getChildByName("CUSTOM_DSP")) {
                customDspCode = dspXml->getAllSubText();
            }
            // ----------------------------------------------

            components.clear();
            if (auto* compsXml = xml->getChildByName("COMPONENTS")) {
                for (auto* cXml : compsXml->getChildIterator()) {
                    Component c;
                    c.index = cXml->getIntAttribute("index");
                    c.type = (ComponentType)cXml->getIntAttribute("type");
                    c.name = cXml->getStringAttribute("name");
                    c.symbol = cXml->getStringAttribute("symbol");
                    c.min = cXml->getDoubleAttribute("min", 0.0);
                    c.max = cXml->getDoubleAttribute("max", 1.0);
                    c.def = cXml->getDoubleAttribute("def", 0.5);
                    c.role = (ParamRole)cXml->getIntAttribute("role", 0);
                    components.push_back(c);
                }
            }
        }
    };
    
    static juce::String getAlgorithmName(AlgorithmType type)
    {
        switch (type)
        {
            case AlgorithmType::Gain:       return "Control de Ganancia (Básico)";
            case AlgorithmType::Distortion: return "Distorsión (Saturación)";
            case AlgorithmType::Tremolo:    return "Trémolo (Modulación AM)";
            case AlgorithmType::Custom:     return "C++ Personalizado (Avanzado)"; 
            default: return "Desconocido";
        }
    }

    static juce::String getDefaultDSPCode(AlgorithmType type)
{
    switch (type)
    {
        case AlgorithmType::Distortion:
            return R"(
float drive = 1.0f;
if (params.size() > 0) drive = params[0];

drive = 1.0f + (drive * 10.0f);

for (int channel = 0; channel < totalNumInputChannels; ++channel)
{
    auto* channelData = buffer.getWritePointer(channel);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        channelData[sample] = std::tanh(channelData[sample] * drive);
    }
}
)";

        case AlgorithmType::Tremolo:
            return R"(
float freq = 1.0f;
float depth = 0.5f;

if (params.size() > 0) freq = params[0];
if (params.size() > 1) depth = params[1];

static float currentPhase = 0.0f;
float sampleRate = getSampleRate();
float phaseIncrement = (freq * 2.0f * juce::MathConstants<float>::pi) / sampleRate;

for (int channel = 0; channel < totalNumInputChannels; ++channel)
{
    auto* channelData = buffer.getWritePointer(channel);
    float phase = currentPhase;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float lfo = 1.0f - (depth * 0.5f * (1.0f + std::sin(phase)));
        channelData[sample] *= lfo;

        phase += phaseIncrement;
        if (phase >= 2.0f * juce::MathConstants<float>::pi)
            phase -= 2.0f * juce::MathConstants<float>::pi;
    }

    if (channel == totalNumInputChannels - 1)
        currentPhase = phase;
}
)";

        case AlgorithmType::Gain:
        default:
            return R"(
float gain = 1.0f;
if (params.size() > 0) gain = params[0];

for (int channel = 0; channel < totalNumInputChannels; ++channel)
{
    auto* channelData = buffer.getWritePointer(channel);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        channelData[sample] *= gain;
    }
}
)";
    }
}
}