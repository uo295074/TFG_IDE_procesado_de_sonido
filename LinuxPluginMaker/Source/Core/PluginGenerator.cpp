/*
  ==============================================================================
    Source/Core/PluginGenerator.cpp
    (VERSIÓN ESTABLE: INIT + VARIABLES + DSP FIX + SELECTOR)
  ==============================================================================
*/

#include "PluginGenerator.h"
#include "Templates.h"

PluginGenerator::PluginGenerator() {
  desktopDir = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                   .getChildFile("MiEfectoDSP");
}

void PluginGenerator::createPluginFiles(PluginData::Project &project) {
  if (!desktopDir.exists())
    desktopDir.createDirectory();

  juce::File sourceDir = desktopDir.getChildFile("Source");
  if (!sourceDir.exists())
    sourceDir.createDirectory();

  // ================================
  // 1. AUDIO PORTS
  // ================================
  juce::String audioPortsTtl = "";
  int portIndex = 0;

  for (int i = 0; i < project.numInputs; ++i) {
    audioPortsTtl += "[ a lv2:InputPort, lv2:AudioPort ; lv2:index " +
                     juce::String(portIndex) + " ; lv2:symbol \\\"in_" +
                     juce::String(i) + "\\\" ; lv2:name \\\"In " +
                     juce::String(i + 1) + "\\\" ]";
    portIndex++;
    if (i < project.numInputs - 1 || project.numOutputs > 0)
      audioPortsTtl += ",\n             ";
  }

  for (int i = 0; i < project.numOutputs; ++i) {
    audioPortsTtl += "[ a lv2:OutputPort, lv2:AudioPort ; lv2:index " +
                     juce::String(portIndex) + " ; lv2:symbol \\\"out_" +
                     juce::String(i) + "\\\" ; lv2:name \\\"Out " +
                     juce::String(i + 1) + "\\\" ]";
    portIndex++;
    if (i < project.numOutputs - 1)
      audioPortsTtl += ",\n             ";
  }

  // ================================
  // 2. CONTROL PORTS
  // ================================
  juce::String ttlPorts = "";

  for (const auto &comp : project.components) {
    ttlPorts += ",\n             [ ";

    if (comp.type == PluginData::ComponentType::Slider ||
        comp.type == PluginData::ComponentType::Knob) {

      ttlPorts += "a lv2:InputPort, lv2:ControlPort ; ";
      ttlPorts += "lv2:index " + juce::String(portIndex) + " ; ";
      ttlPorts += "lv2:symbol \\\"" + comp.symbol + "\\\" ; ";
      ttlPorts += "lv2:name \\\"" + comp.name + "\\\" ; ";
      ttlPorts += "lv2:default " + juce::String(comp.def) + " ; ";
      ttlPorts += "lv2:minimum " + juce::String(comp.min) + " ; ";
      ttlPorts += "lv2:maximum " + juce::String(comp.max);

    } else if (comp.type == PluginData::ComponentType::Toggle) {

      ttlPorts += "a lv2:InputPort, lv2:ControlPort ; ";
      ttlPorts += "lv2:portProperty lv2:toggled ; ";
      ttlPorts += "lv2:index " + juce::String(portIndex) + " ; ";
      ttlPorts += "lv2:symbol \\\"" + comp.symbol + "\\\" ; ";
      ttlPorts += "lv2:name \\\"" + comp.name + "\\\" ; ";
      ttlPorts += "lv2:default " + juce::String(comp.def) + " ; ";
      ttlPorts += "lv2:minimum 0 ; lv2:maximum 1";

    }
    // 🔥 NUEVO: SELECTOR
    else if (comp.type == PluginData::ComponentType::Selector) {

      ttlPorts += "a lv2:InputPort, lv2:ControlPort ; ";
      ttlPorts += "lv2:index " + juce::String(portIndex) + " ; ";
      ttlPorts += "lv2:symbol \\\"" + comp.symbol + "\\\" ; ";
      ttlPorts += "lv2:name \\\"" + comp.name + "\\\" ; ";

      ttlPorts += "lv2:default 0 ; ";
      ttlPorts += "lv2:minimum 0 ; ";
      ttlPorts += "lv2:maximum " + juce::String(comp.numSteps - 1) + " ; ";

      // 🔥 PROPERTIES
      ttlPorts += "lv2:portProperty lv2:integer , lv2:enumeration ; ";

      // 🔥 LABELS (scalePoints)
      if (!comp.options.empty()) {
        ttlPorts += "lv2:scalePoint ";
        for (int i = 0; i < comp.options.size(); ++i) {
          ttlPorts += "[ rdfs:label \\\"" + comp.options[i] +
                      "\\\" ; rdf:value " + juce::String(i) + " ]";
          if (i < comp.options.size() - 1)
            ttlPorts += " , ";
        }
      }
    }

    ttlPorts += " ] ";
    portIndex++;
  }

  // ================================
  // 3. DSP GENERATION
  // ================================
  juce::String dspCode;

  if (project.isCustom) {
    dspCode = project.customDspCode;
  } else {
    switch (project.currentAlgorithm) {

    case PluginData::AlgorithmType::Distortion:
      dspCode = R"(

    float drive = params.size() > 0 ? params[0] : 1.0f;
    float mix   = params.size() > 1 ? params[1] : 1.0f;

    // 🔥 BUSCAR selector (último parámetro normalmente)
    int mode = params.size() > 2 ? (int)params[2] : 0;

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float dry = data[i];
            float x = dry * drive * 5.0f;
            float wet = x;

            switch (mode)
            {
                case 0: // Soft
                    wet = std::tanh(x);
                    break;

                case 1: // Hard
                    wet = juce::jlimit(-1.0f, 1.0f, x);
                    break;

                case 2: // Foldback
                    if (x > 1.0f || x < -1.0f)
                        x = std::fabs(std::fabs(fmod(x - 1.0f, 4.0f)) - 2.0f) - 1.0f;
                    wet = x;
                    break;
            }

            data[i] = dry * (1.0f - mix) + wet * mix;
        }
    }

  )";
      break;

    case PluginData::AlgorithmType::Gain:
      dspCode = R"(

        float gain = params.size() > 0 ? params[0] : 1.0f;

        for (int ch = 0; ch < totalNumInputChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                data[i] *= gain;
            }
        }

    )";
      break;

    default:
      dspCode = R"(

        for (int ch = 0; ch < totalNumInputChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                data[i] *= 1.0f;
            }
        }

    )";
      break;
    }
  }

  // ================================
  // 4. GENERAR ARCHIVOS
  // ================================

  juce::String headerContent = Templates::processorHeader;
  headerContent = headerContent.replace("{{USER_VARS}}", project.userVariables);

  juce::String processorContent = Templates::processorCpp;
  processorContent = processorContent.replace("{{DSP_CODE}}", dspCode);
  processorContent =
      processorContent.replace("{{INIT_CODE}}", project.initCode);

  juce::String editorContent = Templates::editorCpp;
  editorContent =
      editorContent.replace("{{NUM_INPUTS}}", juce::String(project.numInputs));
  editorContent = editorContent.replace("{{NUM_OUTPUTS}}",
                                        juce::String(project.numOutputs));
  editorContent = editorContent.replace(
      "{{NUM_PARAMS}}", juce::String(project.components.size()));

  juce::String cmakeContent = Templates::cmakeFile;
  cmakeContent = cmakeContent.replace("{{AUDIO_PORTS_TTL}}", audioPortsTtl);
  cmakeContent = cmakeContent.replace("{{TTL_PORTS}}", ttlPorts);
  cmakeContent = cmakeContent.replace("{{PLUGIN_NAME}}", project.pluginName);
  cmakeContent = cmakeContent.replace("{{PLUGIN_URI}}", project.pluginURI);

  desktopDir.getChildFile("CMakeLists.txt").replaceWithText(cmakeContent);
  sourceDir.getChildFile("PluginProcessor.h").replaceWithText(headerContent);
  sourceDir.getChildFile("PluginProcessor.cpp")
      .replaceWithText(processorContent);
  sourceDir.getChildFile("PluginEditor.h")
      .replaceWithText(Templates::editorHeader);
  sourceDir.getChildFile("PluginEditor.cpp").replaceWithText(editorContent);
}

// ================================
// COMPILAR E INSTALAR
// ================================
juce::String
PluginGenerator::compileAndInstallPlugin(const PluginData::Project &project) {

  juce::String safeName = "";
  for (auto c : project.pluginName)
    if (juce::CharacterFunctions::isLetter(c))
      safeName += c;

  if (safeName.isEmpty())
    safeName = "MiEfectoDSP";

  juce::String cmd = "cd \"" + desktopDir.getFullPathName() +
                     "\" && "
                     "mkdir -p build && cd build && "
                     "cmake .. && cmake --build . -j4 && "
                     "rm -rf ~/.lv2/" +
                     safeName +
                     ".lv2 && "
                     "mkdir -p ~/.lv2/" +
                     safeName +
                     ".lv2 && "
                     "cp MiEfectoDSP.so manifest.ttl plugin.ttl ~/.lv2/" +
                     safeName + ".lv2/";

  return (system(cmd.toRawUTF8()) == 0) ? "OK:" + safeName
                                        : "ERROR: fallo en compilación";
}

// ================================
// VALIDACIÓN
// ================================
bool PluginGenerator::validateProject(const PluginData::Project &project,
                                      juce::String &error) {

  if (project.components.empty()) {
    error = "Añade al menos un componente.";
    return false;
  }

  std::set<juce::String> symbols;

  for (const auto &comp : project.components) {
    if (symbols.count(comp.symbol)) {
      error = "Hay IDs duplicados (symbol).";
      return false;
    }
    symbols.insert(comp.symbol);
  }

  return true;
}