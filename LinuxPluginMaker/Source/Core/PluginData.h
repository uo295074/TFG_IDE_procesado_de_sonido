#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace PluginData {

// ================================
// ENUMS (mantengo por compatibilidad)
// ================================
enum class AlgorithmType { Gain, Distortion, Tremolo, Custom };

// 🔥 AÑADIDO Selector
enum class ComponentType { Slider, Toggle, Knob, Selector };

// ⚠️ LEGACY (se eliminará en el futuro)
enum class ParamRole { None, Gain, Drive, Mix, Tone, Frequency, Depth };

// ================================
// 🔥 NUEVO SISTEMA DINÁMICO
// ================================
struct EffectParam {
  juce::String name;
};

struct EffectDefinition {
  juce::String name;
  std::vector<EffectParam> params;
};

// ================================
// COMPONENTE
// ================================
struct Component {
  int index;
  ComponentType type;

  juce::String name;
  juce::String symbol;

  // ⚠️ LEGACY (no lo usamos ya)
  ParamRole role = ParamRole::None;

  // 🔥 NUEVO (IMPORTANTE)
  juce::String paramName;

  float min = 0.0f;
  float max = 1.0f;
  float def = 0.5f;

  // 🔥 NUEVO: SELECTOR (discreto)
  int numSteps = 3;

  // 🔥 NUEVO PRO: nombres de opciones (Soft, Hard, etc.)
  std::vector<juce::String> options;
};

// ================================
// PROYECTO
// ================================
struct Project {
  // 🔥 EFECTOS DINÁMICOS DESDE XML
  std::vector<EffectDefinition> availableEffects;
  int currentEffectIndex = 0;

  // 🔥 NUEVO (CLAVE PARA CUSTOM)
  bool isCustom = false;

  juce::String pluginName = "Mi Plugin Nuevo";
  juce::String manufacturer = "Mi Nombre";
  juce::String pluginURI = "http://miweb.com/plugins/miplugin";

  // 🔥 NUEVO: VARIABLES PERSISTENTES
  juce::String userVariables = "// Variables persistentes\n";

  // Código de inicialización
  juce::String initCode;

  // ⚠️ LEGACY
  AlgorithmType currentAlgorithm = AlgorithmType::Gain;

  int numInputs = 2;
  int numOutputs = 2;

  // Código DSP usuario
  juce::String customDspCode =
      "// --- TU CÓDIGO DSP AQUÍ ---\n"
      "for (int channel = 0; channel < totalNumInputChannels; ++channel)\n"
      "{\n"
      "    auto* channelData = buffer.getWritePointer (channel);\n"
      "    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)\n"
      "    {\n"
      "        // Ejemplo: channelData[sample] *= params[0];\n"
      "    }\n"
      "}\n";

  std::vector<Component> components;

  Project() { initCode = "// Código de inicialización\n"; }

  // ================================
  // XML EXPORT
  // ================================
  std::unique_ptr<juce::XmlElement> toXml() const {
    auto xml = std::make_unique<juce::XmlElement>("LINUX_PLUGIN_MAKER_PROJECT");

    xml->setAttribute("name", pluginName);
    xml->setAttribute("manufacturer", manufacturer);
    xml->setAttribute("uri", pluginURI);
    xml->setAttribute("algorithm", (int)currentAlgorithm);

    // 🔥 GUARDAR CUSTOM
    xml->setAttribute("isCustom", isCustom);

    xml->setAttribute("numInputs", numInputs);
    xml->setAttribute("numOutputs", numOutputs);

    // DSP CODE
    auto dspXml = new juce::XmlElement("CUSTOM_DSP");
    dspXml->addTextElement(customDspCode);
    xml->addChildElement(dspXml);

    // INIT CODE
    auto initXml = new juce::XmlElement("INIT_CODE");
    initXml->addTextElement(initCode);
    xml->addChildElement(initXml);

    // 🔥 NUEVO: VARIABLES
    auto varsXml = new juce::XmlElement("USER_VARIABLES");
    varsXml->addTextElement(userVariables);
    xml->addChildElement(varsXml);

    // COMPONENTS
    auto compsXml = new juce::XmlElement("COMPONENTS");

    for (const auto &comp : components) {
      auto cXml = new juce::XmlElement("COMPONENT");

      cXml->setAttribute("index", comp.index);
      cXml->setAttribute("type", (int)comp.type);
      cXml->setAttribute("name", comp.name);
      cXml->setAttribute("symbol", comp.symbol);

      cXml->setAttribute("min", comp.min);
      cXml->setAttribute("max", comp.max);
      cXml->setAttribute("def", comp.def);

      // 🔥 NUEVO (selector)
      cXml->setAttribute("numSteps", comp.numSteps);
      // 🔥 NUEVO PRO: guardar opciones
      if (!comp.options.empty()) {
        juce::String opts;
        for (auto &o : comp.options)
          opts += o + ";";
        cXml->setAttribute("options", opts);
      }

      // legacy
      cXml->setAttribute("role", (int)comp.role);

      // 🔥 NUEVO
      cXml->setAttribute("paramName", comp.paramName);

      compsXml->addChildElement(cXml);
    }

    xml->addChildElement(compsXml);

    return xml;
  }

  // ================================
  // XML IMPORT
  // ================================
  void fromXml(juce::XmlElement *xml) {
    if (!xml || xml->getTagName() != "LINUX_PLUGIN_MAKER_PROJECT")
      return;

    pluginName = xml->getStringAttribute("name", "Mi Plugin Nuevo");
    manufacturer = xml->getStringAttribute("manufacturer", "Mi Nombre");
    pluginURI = xml->getStringAttribute("uri", "");

    currentAlgorithm = (AlgorithmType)xml->getIntAttribute("algorithm", 0);

    // 🔥 CARGAR CUSTOM
    isCustom = xml->getBoolAttribute("isCustom", false);

    numInputs = xml->getIntAttribute("numInputs", 2);
    numOutputs = xml->getIntAttribute("numOutputs", 2);

    // DSP
    if (auto *dspXml = xml->getChildByName("CUSTOM_DSP"))
      customDspCode = dspXml->getAllSubText();

    // INIT
    if (auto *initXml = xml->getChildByName("INIT_CODE"))
      initCode = initXml->getAllSubText();

    // 🔥 NUEVO: VARIABLES
    if (auto *varsXml = xml->getChildByName("USER_VARIABLES"))
      userVariables = varsXml->getAllSubText();

    // COMPONENTS
    components.clear();

    if (auto *compsXml = xml->getChildByName("COMPONENTS")) {
      for (auto *cXml : compsXml->getChildIterator()) {
        Component c;

        c.index = cXml->getIntAttribute("index");
        c.type = (ComponentType)cXml->getIntAttribute("type");

        c.name = cXml->getStringAttribute("name");
        c.symbol = cXml->getStringAttribute("symbol");

        c.min = cXml->getDoubleAttribute("min", 0.0);
        c.max = cXml->getDoubleAttribute("max", 1.0);
        c.def = cXml->getDoubleAttribute("def", 0.5);

        // 🔥 SELECTOR
        c.numSteps = cXml->getIntAttribute("numSteps", 3);

        // 🔥 NUEVO PRO: cargar opciones
        juce::String opts = cXml->getStringAttribute("options", "");
        if (opts.isNotEmpty()) {
          juce::StringArray arr;
          arr.addTokens(opts, ";", "");
          for (auto &o : arr)
            if (o.isNotEmpty())
              c.options.push_back(o);
        }

        // legacy
        c.role = (ParamRole)cXml->getIntAttribute("role", 0);

        // 🔥 NUEVO
        c.paramName = cXml->getStringAttribute("paramName", "");

        components.push_back(c);
      }
    }
  }
};

// ================================
// 🔥 CARGAR EFECTOS DESDE XML
// ================================
static std::vector<EffectDefinition>
loadEffectsFromXML(const juce::File &file) {
  std::vector<EffectDefinition> effects;

  juce::XmlDocument xmlDoc(file);

  if (auto xml = xmlDoc.getDocumentElement()) {
    forEachXmlChildElement(*xml, effectXml) {
      if (effectXml->hasTagName("effect")) {
        EffectDefinition effect;
        effect.name = effectXml->getStringAttribute("name");

        forEachXmlChildElement(*effectXml, paramXml) {
          if (paramXml->hasTagName("param")) {
            EffectParam p;
            p.name = paramXml->getStringAttribute("name");
            effect.params.push_back(p);
          }
        }

        effects.push_back(effect);
      }
    }
  }

  return effects;
}

} // namespace PluginData