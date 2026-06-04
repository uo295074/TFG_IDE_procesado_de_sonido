/*
  ==============================================================================
    Source/Core/PluginGenerator.cpp
    Generacion de proyectos LV2 a partir del estado del IDE.
  ==============================================================================
*/

#include "PluginGenerator.h"
#include "Templates.h"

namespace {
bool isValidJuceDirectory(const juce::File &dir) {
  return dir.isDirectory() && dir.getChildFile("CMakeLists.txt").existsAsFile();
}

void addJuceCandidatesFrom(juce::Array<juce::File> &candidates,
                           const juce::File &baseDir) {
  juce::File dir = baseDir;

  for (int i = 0; i < 8 && dir != juce::File{}; ++i) {
    candidates.addIfNotAlreadyThere(dir.getChildFile("JUCE"));

    auto parent = dir.getParentDirectory();
    if (parent == dir)
      break;

    dir = parent;
  }
}

juce::File findJuceDirectory(const juce::File &startupWorkingDir,
                             juce::StringArray *checkedPaths = nullptr) {
  juce::Array<juce::File> candidates;

  auto appDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                    .getParentDirectory();

  addJuceCandidatesFrom(candidates, appDir);
  addJuceCandidatesFrom(candidates, startupWorkingDir);

  for (auto candidate : candidates) {
    if (checkedPaths != nullptr)
      checkedPaths->add(candidate.getFullPathName());

    if (isValidJuceDirectory(candidate))
      return candidate;
  }

  return {};
}

juce::String findRelevantBuildLine(const juce::String &log) {
  juce::StringArray lines;
  lines.addLines(log);

  for (auto line : lines) {
    auto trimmed = line.trim();
    if (trimmed.containsIgnoreCase("error") ||
        trimmed.containsIgnoreCase("cannot find") ||
        trimmed.containsIgnoreCase("no se puede encontrar") ||
        trimmed.containsIgnoreCase("undefined reference") ||
        trimmed.containsIgnoreCase("permission denied") ||
        trimmed.containsIgnoreCase("command not found"))
      return trimmed;
  }

  return lines.isEmpty() ? "No se pudo leer una linea concreta del error."
                         : lines[juce::jmax(0, lines.size() - 1)].trim();
}

juce::String classifyBuildError(const juce::String &log,
                                const juce::File &logFile) {
  juce::String type = "Error de compilacion no clasificado";
  juce::String suggestion =
      "Revisa el log completo para localizar el origen exacto del fallo.";

  if (log.containsIgnoreCase("cannot find -l") ||
      log.containsIgnoreCase("no se puede encontrar -l") ||
      log.containsIgnoreCase("undefined reference")) {
    type = "Problema de librerias externas";
    suggestion = "Comprueba que la libreria esta instalada y que el campo "
                 "Librerias extra contiene el nombre correcto.";
  } else if (log.containsIgnoreCase("cmake: command not found") ||
             log.containsIgnoreCase("make: command not found") ||
             log.containsIgnoreCase("g++: command not found")) {
    type = "Falta una herramienta del sistema";
    suggestion = "Ejecuta Ayuda > Verificacion de entorno e instala las "
                 "herramientas indicadas.";
  } else if (log.containsIgnoreCase("CMake Error") ||
             log.containsIgnoreCase("Configuring incomplete") ||
             log.containsIgnoreCase("Could not find")) {
    type = "Problema de configuracion CMake";
    suggestion = "Comprueba dependencias, rutas de includes y configuracion "
                 "avanzada de compilacion.";
  } else if (log.containsIgnoreCase("Permission denied") ||
             log.containsIgnoreCase("cannot create directory") ||
             log.containsIgnoreCase("cp:")) {
    type = "Problema de permisos o instalacion";
    suggestion = "Comprueba permisos de escritura en la carpeta generada y en "
                 "~/.lv2.";
  } else if (log.containsIgnoreCase("error:") ||
             log.containsIgnoreCase("fatal error:") ||
             log.containsIgnoreCase("was not declared") ||
             log.containsIgnoreCase("no matching function") ||
             log.containsIgnoreCase("expected")) {
    type = "Problema en codigo C++ o DSP personalizado";
    suggestion = "Revisa el editor DSP, las variables persistentes y el codigo "
                 "de inicializacion.";
  }

  juce::String result;
  result += "ERROR:\n";
  result += "Tipo: " + type + "\n\n";
  result += "Resumen: " + findRelevantBuildLine(log) + "\n\n";
  result += "Sugerencia: " + suggestion + "\n\n";
  result += "Log completo:\n" + logFile.getFullPathName();
  return result;
}
} // namespace

PluginGenerator::PluginGenerator() {
  startupWorkingDir = juce::File::getCurrentWorkingDirectory();
  desktopDir =
      startupWorkingDir.getChildFile("GeneratedPlugins");
}

juce::String
PluginGenerator::getSafePluginFileName(const PluginData::Project &project) {
  juce::String safeName;

  for (auto c : project.pluginName) {
    if (juce::CharacterFunctions::isLetterOrDigit(c))
      safeName += c;
    else if (c == ' ' || c == '-' || c == '_')
      safeName += "_";
  }

  while (safeName.contains("__"))
    safeName = safeName.replace("__", "_");

  safeName = safeName.trimCharactersAtStart("_").trimCharactersAtEnd("_");

  if (safeName.isEmpty())
    safeName = "MiEfectoDSP";

  if (!juce::CharacterFunctions::isLetter(safeName[0]))
    safeName = "Plugin_" + safeName;

  return safeName;
}

juce::File
PluginGenerator::getGeneratedProjectDir(const PluginData::Project &project) const {
  return desktopDir.getChildFile(getSafePluginFileName(project));
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

// Mantiene el parametro en un rango seguro para el filtro.
cutoffNorm = juce::jlimit(0.001f, 0.999f, cutoffNorm);

// Mapeo aproximado de 20 Hz a 20 kHz.
float cutoff = 20.0f * std::pow(1000.0f, cutoffNorm);

// Coeficiente suavizado para evitar saltos bruscos.
float x = std::exp(-2.0f * juce::MathConstants<float>::pi * cutoff / getSampleRate());

// Estado persistente por canal.
static float zL = 0.0f;
static float zR = 0.0f;

for (int ch = 0; ch < totalNumInputChannels; ++ch)
{
    auto* data = buffer.getWritePointer(ch);

    float& z = (ch == 0) ? zL : zR;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float in = data[i];

        // Filtro low-pass base a partir del que se derivan los otros modos.
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

// Limita la profundidad de modulacion.
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

// Buffers fijos para mantener el estado de la cola de reverberacion.
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

  // Efecto de delay predefinido.
  case PluginData::AlgorithmType::Delay:
    return R"(

auto getParam = [&](int index, float def)
{
    return (index < (int)params.size()) ? params[index] : def;
};

float timeNorm = getParam(0, 0.5f);
float feedback = getParam(1, 0.35f);
float mix = getParam(2, 0.35f);

timeNorm = juce::jlimit(0.0f, 1.0f, timeNorm);
feedback = juce::jlimit(0.0f, 0.95f, feedback);
mix = juce::jlimit(0.0f, 1.0f, mix);

constexpr int maxDelaySamples = 96000;
float delayMs = 10.0f + timeNorm * 790.0f;
int delaySamples = (int)((delayMs / 1000.0f) * getSampleRate());
delaySamples = juce::jlimit(1, maxDelaySamples - 1, delaySamples);

static float delayBufferL[maxDelaySamples] = {0.0f};
static float delayBufferR[maxDelaySamples] = {0.0f};
static int writeIndex = 0;

int localWriteIndex = writeIndex;

for (int ch = 0; ch < totalNumInputChannels; ++ch)
{
    auto* data = buffer.getWritePointer(ch);
    float* delayBuffer = (ch == 0) ? delayBufferL : delayBufferR;
    int channelWriteIndex = localWriteIndex;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        int readIndex = channelWriteIndex - delaySamples;
        if (readIndex < 0)
            readIndex += maxDelaySamples;

        float input = data[i];
        float delayed = delayBuffer[readIndex];

        delayBuffer[channelWriteIndex] = input + delayed * feedback;
        data[i] = input * (1.0f - mix) + delayed * mix;

        channelWriteIndex++;
        if (channelWriteIndex >= maxDelaySamples)
            channelWriteIndex = 0;
    }
}

writeIndex = localWriteIndex + buffer.getNumSamples();
while (writeIndex >= maxDelaySamples)
    writeIndex -= maxDelaySamples;

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

  juce::File projectDir = getGeneratedProjectDir(project);
  if (projectDir.exists())
    projectDir.deleteRecursively();
  projectDir.createDirectory();

  juce::File sourceDir = projectDir.getChildFile("Source");
  if (!sourceDir.exists())
    sourceDir.createDirectory();

  juce::File juceSource = findJuceDirectory(startupWorkingDir);

  // Puertos de audio declarados en el descriptor LV2.
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

  // Puertos de control creados a partir de los componentes del lienzo.
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
    else if (comp.type == PluginData::ComponentType::Selector) {

      ttlPorts += "a lv2:InputPort, lv2:ControlPort ; ";
      ttlPorts += "lv2:index " + juce::String(portIndex) + " ; ";
      ttlPorts += "lv2:symbol \\\"" + comp.symbol + "\\\" ; ";
      ttlPorts += "lv2:name \\\"" + comp.name + "\\\" ; ";

      ttlPorts += "lv2:default 0 ; ";
      ttlPorts += "lv2:minimum 0 ; ";
      ttlPorts += "lv2:maximum " + juce::String(comp.numSteps - 1) + " ; ";

      ttlPorts += "lv2:portProperty lv2:integer , lv2:enumeration ; ";

      // scalePoint permite que el host muestre nombres legibles para cada opcion.
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

        ttlPorts += " ; "; // Cierra la lista de scalePoint dentro del puerto.
      }
    }

    ttlPorts += " ] ";
    portIndex++;
  }

  // Bypass global: se añade como control extra, separado de los parametros DSP.
  int numUserParams = (int)project.components.size();
  int numControlParams = numUserParams + (project.enableBypass ? 1 : 0);
  juce::String bypassDefines = "#define HAS_BYPASS " +
                               juce::String(project.enableBypass ? 1 : 0) +
                               "\n#define BYPASS_PARAM_INDEX " +
                               juce::String(project.enableBypass ? numUserParams : -1) +
                               "\n";

  if (project.enableBypass) {
    ttlPorts += ",\n             [ a lv2:InputPort, lv2:ControlPort ; ";
    ttlPorts += "lv2:portProperty lv2:toggled ; ";
    ttlPorts += "lv2:index " + juce::String(portIndex++) + " ; ";
    ttlPorts += "lv2:symbol \\\"bypass\\\" ; lv2:name \\\"Bypass\\\" ; ";
    ttlPorts += "lv2:default 0 ; lv2:minimum 0 ; lv2:maximum 1 ] ";
  }

  // Indicadores de monitorizacion expuestos como puertos LV2 de salida.
  struct MonitorPort {
    juce::String symbol;
    juce::String name;
    juce::String defineName;
  };

  std::vector<MonitorPort> monitors;

  auto addMonitor = [&monitors](bool enabled, const juce::String &symbol,
                                const juce::String &name,
                                const juce::String &defineName) {
    if (enabled)
      monitors.push_back({symbol, name, defineName});
  };

  addMonitor(project.enableInputLed, "input_led", "Input LED",
             "MONITOR_INPUT_LED");
  addMonitor(project.enableOutputLed, "output_led", "Output LED",
             "MONITOR_OUTPUT_LED");
  addMonitor(project.enableClipLed, "clip_led", "Clip LED", "MONITOR_CLIP_LED");
  addMonitor(project.enableLevelMeter, "level_meter", "Level Meter",
             "MONITOR_LEVEL_METER");
  addMonitor(project.enableRmsMeter, "rms_meter", "RMS Meter",
             "MONITOR_RMS_METER");
  addMonitor(project.enableProcessingLed, "processing_led", "Processing LED",
             "MONITOR_PROCESSING_LED");

  juce::String monitorTtlPorts;
  juce::String monitorDefines;

  for (int i = 0; i < (int)monitors.size(); ++i) {
    const auto &monitor = monitors[(size_t)i];

    monitorTtlPorts += ",\n             [ a lv2:OutputPort, lv2:ControlPort ; ";
    monitorTtlPorts += "lv2:index " + juce::String(portIndex++) + " ; ";
    monitorTtlPorts += "lv2:symbol \\\"" + monitor.symbol + "\\\" ; ";
    monitorTtlPorts += "lv2:name \\\"" + monitor.name + "\\\" ; ";
    monitorTtlPorts += "lv2:default 0 ; lv2:minimum 0 ; lv2:maximum 1 ] ";

    monitorDefines += "#define " + monitor.defineName + " " + juce::String(i) + "\n";
  }

  int numMonitorPorts = (int)monitors.size();

  // Selecciona el codigo DSP: personalizado o predefinido.
  juce::String dspCode;

  if (project.isCustom) {
    dspCode = project.customDspCode;
  } else {
    dspCode = getBuiltinDspCode(project.currentAlgorithm);
  }

  // Rellena las plantillas con los datos del proyecto.

  juce::String headerContent = Templates::processorHeader;
  headerContent = headerContent.replace("{{EXTRA_HEADERS}}",
                                        project.extraHeaders.trim());
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
  editorContent =
      editorContent.replace("{{NUM_USER_PARAMS}}", juce::String(numUserParams));
  editorContent = editorContent.replace("{{NUM_CONTROL_PARAMS}}",
                                        juce::String(numControlParams));
  editorContent = editorContent.replace("{{BYPASS_DEFINES}}", bypassDefines);
  editorContent =
      editorContent.replace("{{NUM_MONITOR_PORTS}}", juce::String(numMonitorPorts));
  editorContent = editorContent.replace("{{MONITOR_DEFINES}}", monitorDefines);

  // Nombres de parametros usados por el wrapper para mantener el orden visible.
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

  // Evita dejar una coma final en la lista generada.
  if (paramNamesStr.endsWith(", "))
    paramNamesStr = paramNamesStr.dropLastCharacters(2);

  editorContent = editorContent.replace("{{PARAM_NAMES}}", paramNamesStr);

  // Rutas de cabecera adicionales introducidas por el usuario.
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
  juce::String binaryName = getSafePluginFileName(project);
  juce::String juceSourcePath = juceSource.getFullPathName().replace("\\", "/");
  cmakeContent = cmakeContent.replace("{{AUDIO_PORTS_TTL}}", audioPortsTtl);
  cmakeContent = cmakeContent.replace("{{TTL_PORTS}}", ttlPorts);
  cmakeContent = cmakeContent.replace("{{PLUGIN_NAME}}", project.pluginName);
  cmakeContent = cmakeContent.replace("{{PLUGIN_URI}}", project.pluginURI);
  cmakeContent = cmakeContent.replace("{{PLUGIN_BINARY_NAME}}", binaryName);
  cmakeContent = cmakeContent.replace("{{JUCE_SOURCE_DIR}}", juceSourcePath);
  cmakeContent = cmakeContent.replace("{{MONITOR_TTL_PORTS}}", monitorTtlPorts);
  cmakeContent =
      cmakeContent.replace("{{EXTRA_INCLUDE_DIRS}}", extraIncludesCmake);
  cmakeContent = cmakeContent.replace("{{EXTRA_LIBRARIES}}", extraLibrariesCmake);

  projectDir.getChildFile("CMakeLists.txt").replaceWithText(cmakeContent);
  sourceDir.getChildFile("PluginProcessor.h").replaceWithText(headerContent);
  sourceDir.getChildFile("PluginProcessor.cpp")
      .replaceWithText(processorContent);
  sourceDir.getChildFile("PluginEditor.h")
      .replaceWithText(Templates::editorHeader);
  sourceDir.getChildFile("PluginEditor.cpp").replaceWithText(editorContent);
}

// Compila el proyecto generado y lo instala en la carpeta LV2 del usuario.
juce::String
PluginGenerator::compileAndInstallPlugin(const PluginData::Project &project) {

  juce::String safeName = getSafePluginFileName(project);
  juce::File projectDir = getGeneratedProjectDir(project);
  juce::File logFile = projectDir.getChildFile("build").getChildFile("build.log");
  juce::StringArray checkedJucePaths;
  juce::File juceSource = findJuceDirectory(startupWorkingDir, &checkedJucePaths);

  if (!isValidJuceDirectory(juceSource)) {
    juce::String result;
    result += "ERROR:\n";
    result += "Tipo: Problema de configuracion CMake\n\n";
    result += "Resumen: No se encontro una carpeta JUCE valida.\n\n";
    result += "Sugerencia: Comprueba que la carpeta JUCE esta junto al ejecutable del IDE o en alguno de sus directorios padre.\n\n";
    result += "Rutas comprobadas:\n" + checkedJucePaths.joinIntoString("\n");
    return result;
  }

  juce::String cmd = "cd \"" + projectDir.getFullPathName() +
                     "\" && "
                     "mkdir -p build && rm -f build/build.log && cd build && "
                     "(cmake .. && cmake --build . -j4 && "
                     "rm -rf ~/.lv2/" +
                     safeName +
                     ".lv2 && "
                     "mkdir -p ~/.lv2/" +
                     safeName +
                     ".lv2 && "
                     "cp " + safeName + ".so manifest.ttl plugin.ttl ~/.lv2/" +
                     safeName + ".lv2/) > build.log 2>&1";

  if (system(cmd.toRawUTF8()) == 0)
    return "OK:" + safeName;

  juce::String log = logFile.existsAsFile() ? logFile.loadFileAsString()
                                            : "No se encontro build.log.";
  return classifyBuildError(log, logFile);
}

// Comprueba condiciones minimas antes de generar el proyecto.
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
