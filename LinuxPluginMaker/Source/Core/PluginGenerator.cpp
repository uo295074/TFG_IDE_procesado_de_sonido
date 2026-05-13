/*
  ==============================================================================
    Source/Core/PluginGenerator.cpp
    (VERSIÓN ESTABLE: INIT + VARIABLES + DSP FIX + SELECTOR)
  ==============================================================================
*/

#include "PluginGenerator.h"
#include "Templates.h"

PluginGenerator::PluginGenerator() {
  desktopDir =
      juce::File::getCurrentWorkingDirectory().getChildFile("GeneratedPlugins");
}

juce::String
PluginGenerator::getBuiltinDspCode(PluginData::AlgorithmType algorithm) {
  switch (algorithm) {
  case PluginData::AlgorithmType::Distortion:
    return R"(

    auto getParam = [&](int index, float def)
{
    return (index < (int)params.size()) ? params[index] : def;
};
    float drive = params.size() > 0 ? params[0] : 1.0f;
    float mix   = params.size() > 1 ? params[1] : 1.0f;

    int modeIndex = (int)params.size() - 1;
    int mode = modeIndex >= 0 ? (int)params[modeIndex] : 0;

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
                case 0: wet = std::tanh(x); break;
                case 1: wet = juce::jlimit(-1.0f, 1.0f, x); break;
                case 2:
                    if (x > 1.0f || x < -1.0f)
                        x = std::fabs(std::fabs(fmod(x - 1.0f, 4.0f)) - 2.0f) - 1.0f;
                    wet = x;
                    break;
            }

            data[i] = dry * (1.0f - mix) + wet * mix;
        }
    }

)";

  case PluginData::AlgorithmType::Gain:
    return R"(

     auto getParam = [&](int index, float def)
{
    return (index < (int)params.size()) ? params[index] : def;
};
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

  case PluginData::AlgorithmType::Filter:
    return R"(

auto getParam = [&](int index, float def)
{
    return (index < (int)params.size()) ? params[index] : def;
};

float cutoffNorm = getParam(0, 0.5f);
int mode = (int)getParam(params.size() - 1, 0.0f);

// 🔥 limitar rango útil
cutoffNorm = juce::jlimit(0.001f, 0.999f, cutoffNorm);

// 🔥 mapeo log correcto (20Hz - 20kHz)
float cutoff = 20.0f * std::pow(1000.0f, cutoffNorm);

// 🔥 coeficiente estable
float x = std::exp(-2.0f * juce::MathConstants<float>::pi * cutoff / getSampleRate());

// 🔥 estado persistente por canal
static float zL = 0.0f;
static float zR = 0.0f;

for (int ch = 0; ch < totalNumInputChannels; ++ch)
{
    auto* data = buffer.getWritePointer(ch);

    float& z = (ch == 0) ? zL : zR;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float in = data[i];

        // 🔥 low-pass base
        z = (1.0f - x) * in + x * z;

        float out = in;

        switch (mode)
        {
            case 0: out = z; break;           // Low-pass
            case 1: out = in - z; break;      // High-pass
            case 2: out = (in - z) * z; break; // pseudo band-pass
        }

        data[i] = out;
    }
}

)";

  case PluginData::AlgorithmType::Tremolo:
    return R"(

      auto getParam = [&](int index, float def)
{
    return (index < (int)params.size()) ? params[index] : def;
};
    float rateNorm  = getParam(0, 0.5f);
float depth     = getParam(1, 0.5f);
int mode        = (int)getParam(params.size() - 1, 0.0f);

// 🔥 evitar valores locos
depth = juce::jlimit(0.0f, 1.0f, depth);

float rate = 0.1f + rateNorm * 10.0f;

float phase = 0.0f;
float phaseInc = 2.0f * juce::MathConstants<float>::pi * rate / getSampleRate();

for (int ch = 0; ch < totalNumInputChannels; ++ch)
{
    auto* data = buffer.getWritePointer(ch);
    float p = phase;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float lfo = 0.0f;

        switch (mode)
        {
            case 0: lfo = 0.5f * (1.0f + std::sin(p)); break;
            case 1: lfo = (std::sin(p) > 0.0f) ? 1.0f : 0.0f; break;
            case 2: lfo = std::abs(std::sin(p)); break;
        }

        float mod = 1.0f - depth + depth * lfo;

        data[i] *= mod;

        p += phaseInc;
        if (p > 2.0f * juce::MathConstants<float>::pi)
            p -= 2.0f * juce::MathConstants<float>::pi;
    }
}

)";

  case PluginData::AlgorithmType::Reverb:
    return R"(

auto getParam = [&](int index, float def)
{
    return (index < (int)params.size()) ? params[index] : def;
};

float mix   = getParam(0, 0.5f);
float decay = getParam(1, 0.5f);

mix   = juce::jlimit(0.0f, 1.0f, mix);
decay = juce::jlimit(0.0f, 0.95f, decay);

const int delaySamples = (int)(0.2f * getSampleRate());

// 🔥 buffers fijos (SIN vector)
static float delayBufferL[48000] = {0}; // hasta 1s aprox
static float delayBufferR[48000] = {0};

static int writeIndex = 0;

for (int ch = 0; ch < totalNumInputChannels; ++ch)
{
    auto* data = buffer.getWritePointer(ch);

    float* delayBuffer = (ch == 0) ? delayBufferL : delayBufferR;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float in = data[i];

        float delayed = delayBuffer[writeIndex];

        float out = in * (1.0f - mix) + delayed * mix;

        delayBuffer[writeIndex] = in + delayed * decay;

        data[i] = out;

        writeIndex++;
        if (writeIndex >= delaySamples)
            writeIndex = 0;
    }
}

)";

  default:
    return R"(

        for (int ch = 0; ch < totalNumInputChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                data[i] *= 1.0f;
            }
        }

    )";
  }
}

void PluginGenerator::createPluginFiles(PluginData::Project &project) {
  if (!desktopDir.exists())
    desktopDir.createDirectory();

  juce::File sourceDir = desktopDir.getChildFile("Source");
  if (!sourceDir.exists())
    sourceDir.createDirectory();

  juce::File juceSource =
      juce::File::getSpecialLocation(juce::File::currentApplicationFile)
          .getParentDirectory()
          .getChildFile("JUCE");

  juce::File juceDest = desktopDir.getChildFile("JUCE");

  if (juceSource.exists()) {
    juceDest.deleteRecursively();
    juceSource.copyDirectoryTo(juceDest);
  }

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

      // 🔥🔥🔥 FIX REAL: scalePoints bien formateados
      if (!comp.options.empty()) {
        ttlPorts += "lv2:scalePoint ";

        for (int i = 0; i < comp.options.size(); ++i) {
          ttlPorts += "[ ";
          ttlPorts += "rdfs:label \\\"" + comp.options[i] + "\\\" ; ";
          ttlPorts += "rdf:value " + juce::String(i) + " ";
          ttlPorts += "]";

          if (i < comp.options.size() - 1)
            ttlPorts += " , ";
        }

        ttlPorts += " ; "; // 🔥 IMPORTANTE (no coma, no punto)
      }
    }

    ttlPorts += " ] ";
    portIndex++;
  }

  // 🔥 SIGNAL INDICATORS
  juce::String monitorTtlPorts;
  int numMonitorPorts = 0;

  if (project.enableSignalIndicators) {
    monitorTtlPorts += ",\n             [ a lv2:OutputPort, lv2:ControlPort ; ";
    monitorTtlPorts += "lv2:index " + juce::String(portIndex++) + " ; ";
    monitorTtlPorts +=
        "lv2:symbol \\\"signal_led\\\" ; lv2:name \\\"Signal LED\\\" ; ";
    monitorTtlPorts += "lv2:minimum 0 ; lv2:maximum 1 ; lv2:default 0 ] ";

    monitorTtlPorts += ",\n             [ a lv2:OutputPort, lv2:ControlPort ; ";
    monitorTtlPorts += "lv2:index " + juce::String(portIndex++) + " ; ";
    monitorTtlPorts +=
        "lv2:symbol \\\"clip_led\\\" ; lv2:name \\\"Clip LED\\\" ; ";
    monitorTtlPorts += "lv2:minimum 0 ; lv2:maximum 1 ; lv2:default 0 ] ";

    monitorTtlPorts += ",\n             [ a lv2:OutputPort, lv2:ControlPort ; ";
    monitorTtlPorts += "lv2:index " + juce::String(portIndex++) + " ; ";
    monitorTtlPorts +=
        "lv2:symbol \\\"level_meter\\\" ; lv2:name \\\"Level Meter\\\" ; ";
    monitorTtlPorts += "lv2:minimum 0 ; lv2:maximum 1 ; lv2:default 0 ] ";

    numMonitorPorts = 3;
  }

  // ================================
  // 3. DSP GENERATION
  // ================================
  juce::String dspCode;

  if (project.isCustom) {
    dspCode = project.customDspCode;
  } else {
    dspCode = getBuiltinDspCode(project.currentAlgorithm);
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
  editorContent =
      editorContent.replace("{{NUM_MONITOR_PORTS}}", juce::String(numMonitorPorts));

  // 🔥🔥🔥 GENERAR PARAM_NAMES
  juce::String paramNamesStr;

  for (const auto &comp : project.components) {
    if (comp.type == PluginData::ComponentType::Slider ||
        comp.type == PluginData::ComponentType::Knob ||
        comp.type == PluginData::ComponentType::Selector) {
      juce::String name =
          comp.paramName.isNotEmpty() ? comp.paramName : comp.symbol;
      paramNamesStr += "\"" + name + "\", ";
    }
  }

  // 🔥 IMPORTANTE: quitar última coma
  if (paramNamesStr.endsWith(", "))
    paramNamesStr = paramNamesStr.dropLastCharacters(2);

  editorContent = editorContent.replace("{{PARAM_NAMES}}", paramNamesStr);

  // 🔥 EXTRA BUILD CONFIG
  juce::String extraIncludesCmake;
  {
    juce::StringArray lines;
    lines.addLines(project.extraIncludePaths);

    for (auto line : lines) {
      auto cleaned = line.trim();
      if (cleaned.isNotEmpty())
        extraIncludesCmake += "include_directories(\"" + cleaned + "\")\n";
    }
  }

  juce::String extraLibrariesCmake;
  {
    juce::StringArray libs;
    libs.addTokens(project.extraLibraries, " ,\n\r\t", "");
    libs.removeEmptyStrings();
    libs.trim();

    for (int i = 0; i < libs.size(); ++i) {
      extraLibrariesCmake += libs[i];
      if (i < libs.size() - 1)
        extraLibrariesCmake += " ";
    }
  }

  juce::String cmakeContent = Templates::cmakeFile;
  cmakeContent = cmakeContent.replace("{{AUDIO_PORTS_TTL}}", audioPortsTtl);
  cmakeContent = cmakeContent.replace("{{TTL_PORTS}}", ttlPorts);
  cmakeContent = cmakeContent.replace("{{PLUGIN_NAME}}", project.pluginName);
  cmakeContent = cmakeContent.replace("{{PLUGIN_URI}}", project.pluginURI);
  cmakeContent = cmakeContent.replace("{{MONITOR_TTL_PORTS}}", monitorTtlPorts);
  cmakeContent =
      cmakeContent.replace("{{EXTRA_INCLUDE_DIRS}}", extraIncludesCmake);
  cmakeContent = cmakeContent.replace("{{EXTRA_LIBRARIES}}", extraLibrariesCmake);

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
