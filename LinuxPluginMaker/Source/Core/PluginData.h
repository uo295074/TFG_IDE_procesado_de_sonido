#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace PluginData {

// Tipos basicos que describen los algoritmos y controles disponibles en el IDE.
enum class AlgorithmType { Gain, Distortion, Filter, Tremolo, Reverb, Delay, Custom };

enum class ComponentType { Slider, Toggle, Knob, Selector };

// Campo mantenido para poder abrir proyectos antiguos que usaban roles fijos.
enum class ParamRole { None, Gain, Drive, Mix, Tone, Frequency, Depth };

// Definicion minima de un parametro cargado desde effects.xml.
struct EffectParam {
  juce::String name;
};

// Selectores discretos definidos en effects.xml, por ejemplo modos de filtro.
struct EffectSelector {
  juce::String name;
  std::vector<juce::String> options;
};

// Descripcion de un efecto disponible en el desplegable del IDE.
struct EffectDefinition {
  juce::String name;
  std::vector<EffectParam> params;

  std::vector<EffectSelector> selectors;
};

// Componente visual colocado en el lienzo y convertido despues en puerto LV2.
struct Component {
  int index;
  ComponentType type;

  juce::String name;
  juce::String symbol;

  // Se conserva para compatibilidad con proyectos guardados en versiones previas.
  ParamRole role = ParamRole::None;

  // Nombre del parametro DSP al que queda enlazado el control.
  juce::String paramName;

  float min = 0.0f;
  float max = 1.0f;
  float def = 0.5f;

  // Datos usados por los controles discretos de tipo selector.
  int numSteps = 3;
  std::vector<juce::String> options;
};

// Estado completo de un proyecto del IDE.
struct Project {
  std::vector<EffectDefinition> availableEffects;
  int currentEffectIndex = 0;

  bool isCustom = false;

  juce::String pluginName = "Mi Plugin Nuevo";
  juce::String manufacturer = "Mi Nombre";
  juce::String pluginURI = "http://miweb.com/plugins/miplugin";

  // Configuracion avanzada que se insertara en los archivos generados.
  juce::String extraHeaders;
  juce::String extraLibraries;
  juce::String extraIncludePaths;

  juce::String userVariables = "// Variables persistentes\n";
  juce::String initCode;

  AlgorithmType currentAlgorithm = AlgorithmType::Gain;

  int numInputs = 2;
  int numOutputs = 2;

  // Bypass global generado como puerto de control independiente.
  bool enableBypass = false;

  // Indicadores de monitorizacion generados como puertos LV2 de salida.
  bool enableInputLed = false;
  bool enableOutputLed = false;
  bool enableClipLed = true;
  bool enableLevelMeter = true;
  bool enableRmsMeter = false;
  bool enableProcessingLed = false;

  juce::String customDspCode = juce::String::fromUTF8(
      "// --- TU CÓDIGO DSP AQUÍ ---\n"
      "for (int channel = 0; channel < totalNumInputChannels; ++channel)\n"
      "{\n"
      "    auto* channelData = buffer.getWritePointer (channel);\n"
      "    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)\n"
      "    {\n"
      "    }\n"
      "}\n");

  std::vector<Component> components;

  Project() {
    initCode = juce::String::fromUTF8("// Código de inicialización\n");
  }

  // Serializa el proyecto completo para poder recuperarlo en otra sesion.
  std::unique_ptr<juce::XmlElement> toXml() const {
    auto xml = std::make_unique<juce::XmlElement>("LINUX_PLUGIN_MAKER_PROJECT");

    xml->setAttribute("name", pluginName);
    xml->setAttribute("manufacturer", manufacturer);
    xml->setAttribute("uri", pluginURI);
    xml->setAttribute("algorithm", (int)currentAlgorithm);
    xml->setAttribute("effectIndex", currentEffectIndex);

    xml->setAttribute("isCustom", isCustom);
    xml->setAttribute("numInputs", numInputs);
    xml->setAttribute("numOutputs", numOutputs);
    xml->setAttribute("enableBypass", enableBypass);
    xml->setAttribute("enableInputLed", enableInputLed);
    xml->setAttribute("enableOutputLed", enableOutputLed);
    xml->setAttribute("enableClipLed", enableClipLed);
    xml->setAttribute("enableLevelMeter", enableLevelMeter);
    xml->setAttribute("enableRmsMeter", enableRmsMeter);
    xml->setAttribute("enableProcessingLed", enableProcessingLed);

    auto dspXml = new juce::XmlElement("CUSTOM_DSP");
    dspXml->addTextElement(customDspCode);
    xml->addChildElement(dspXml);

    auto initXml = new juce::XmlElement("INIT_CODE");
    initXml->addTextElement(initCode);
    xml->addChildElement(initXml);

    auto varsXml = new juce::XmlElement("USER_VARIABLES");
    varsXml->addTextElement(userVariables);
    xml->addChildElement(varsXml);

    auto extraHeadersXml = new juce::XmlElement("EXTRA_HEADERS");
    extraHeadersXml->addTextElement(extraHeaders);
    xml->addChildElement(extraHeadersXml);

    auto extraLibsXml = new juce::XmlElement("EXTRA_LIBRARIES");
    extraLibsXml->addTextElement(extraLibraries);
    xml->addChildElement(extraLibsXml);

    auto extraIncludesXml = new juce::XmlElement("EXTRA_INCLUDE_PATHS");
    extraIncludesXml->addTextElement(extraIncludePaths);
    xml->addChildElement(extraIncludesXml);

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

      cXml->setAttribute("numSteps", comp.numSteps);

      if (!comp.options.empty()) {
        juce::String opts;
        for (auto &o : comp.options)
          opts += o + ";";
        cXml->setAttribute("options", opts);
      }

      cXml->setAttribute("role", (int)comp.role);
      cXml->setAttribute("paramName", comp.paramName);

      compsXml->addChildElement(cXml);
    }

    xml->addChildElement(compsXml);
    return xml;
  }

  // Restaura el estado del proyecto a partir del XML guardado por el IDE.
  void fromXml(juce::XmlElement *xml) {
    if (!xml || xml->getTagName() != "LINUX_PLUGIN_MAKER_PROJECT")
      return;

    pluginName = xml->getStringAttribute("name", "Mi Plugin Nuevo");
    manufacturer = xml->getStringAttribute("manufacturer", "Mi Nombre");
    pluginURI = xml->getStringAttribute("uri", "");

    currentAlgorithm = (AlgorithmType)xml->getIntAttribute("algorithm", 0);
    isCustom = xml->getBoolAttribute("isCustom", false);
    currentEffectIndex = xml->getIntAttribute("effectIndex", (int)currentAlgorithm);

    if (currentEffectIndex < 0)
      currentEffectIndex = 0;

    if (!isCustom && currentEffectIndex >= (int)availableEffects.size())
      currentEffectIndex = 0;

    numInputs = xml->getIntAttribute("numInputs", 2);
    numOutputs = xml->getIntAttribute("numOutputs", 2);
    enableBypass = xml->getBoolAttribute("enableBypass", false);
    // Los proyectos antiguos solo tenian un interruptor general de indicadores.
    bool oldIndicatorsEnabled =
        xml->getBoolAttribute("enableSignalIndicators", true);
    enableInputLed = xml->getBoolAttribute("enableInputLed", false);
    enableOutputLed = xml->getBoolAttribute("enableOutputLed", false);
    enableClipLed = xml->getBoolAttribute("enableClipLed", oldIndicatorsEnabled);
    enableLevelMeter =
        xml->getBoolAttribute("enableLevelMeter", oldIndicatorsEnabled);
    enableRmsMeter = xml->getBoolAttribute("enableRmsMeter", false);
    enableProcessingLed = xml->getBoolAttribute("enableProcessingLed", false);

    if (auto *dspXml = xml->getChildByName("CUSTOM_DSP"))
      customDspCode = dspXml->getAllSubText();

    if (auto *initXml = xml->getChildByName("INIT_CODE"))
      initCode = initXml->getAllSubText();

    if (auto *varsXml = xml->getChildByName("USER_VARIABLES"))
      userVariables = varsXml->getAllSubText();

    extraHeaders.clear();
    extraLibraries.clear();
    extraIncludePaths.clear();

    if (auto *extraHeadersXml = xml->getChildByName("EXTRA_HEADERS"))
      extraHeaders = extraHeadersXml->getAllSubText();

    if (auto *extraLibsXml = xml->getChildByName("EXTRA_LIBRARIES"))
      extraLibraries = extraLibsXml->getAllSubText();

    if (auto *extraIncludesXml = xml->getChildByName("EXTRA_INCLUDE_PATHS"))
      extraIncludePaths = extraIncludesXml->getAllSubText();

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

        c.numSteps = cXml->getIntAttribute("numSteps", 3);

        juce::String opts = cXml->getStringAttribute("options", "");
        if (opts.isNotEmpty()) {
          juce::StringArray arr;
          arr.addTokens(opts, ";", "");
          for (auto &o : arr)
            if (o.isNotEmpty())
              c.options.push_back(o);
        }

        c.role = (ParamRole)cXml->getIntAttribute("role", 0);
        c.paramName = cXml->getStringAttribute("paramName", "");

        components.push_back(c);
      }
    }
  }
};

// Carga los efectos disponibles desde effects.xml para no tenerlos fijados en codigo.
static std::vector<EffectDefinition>
loadEffectsFromXML(const juce::File &file) {
  std::vector<EffectDefinition> effects;

  juce::XmlDocument xmlDoc(file);

  if (auto xml = xmlDoc.getDocumentElement()) {
    forEachXmlChildElement(*xml, effectXml) {
      if (effectXml->hasTagName("effect")) {
        EffectDefinition effect;
        effect.name = effectXml->getStringAttribute("name");

        forEachXmlChildElement(*effectXml, childXml) {

          // Parametros continuos del efecto.
          if (childXml->hasTagName("param")) {
            EffectParam p;
            p.name = childXml->getStringAttribute("name");
            effect.params.push_back(p);
          }

          // Selectores discretos, como modos o tipos de procesado.
          if (childXml->hasTagName("selector")) {
            EffectSelector sel;
            sel.name = childXml->getStringAttribute("name");

            forEachXmlChildElement(*childXml, optXml) {
              if (optXml->hasTagName("option")) {
                sel.options.push_back(optXml->getAllSubText());
              }
            }

            effect.selectors.push_back(sel);
          }
        }

        effects.push_back(effect);
      }
    }
  }

  return effects;
}

} // namespace PluginData
