/*
  ==============================================================================
    Source/Core/PluginGenerator.cpp
    (VERSIÓN MEDIO FINAL: Mapeo por ParamRole)
  ==============================================================================
*/

#include "PluginGenerator.h"
#include "Templates.h"

PluginGenerator::PluginGenerator()
{
    desktopDir = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                 .getChildFile("MiEfectoDSP");
}

void PluginGenerator::createPluginFiles(PluginData::Project& project)
{
    if (!desktopDir.exists()) desktopDir.createDirectory();
    juce::File sourceDir = desktopDir.getChildFile("Source");
    if (!sourceDir.exists()) sourceDir.createDirectory();

    // ================================
    // 1. AUDIO PORTS
    // ================================
    juce::String audioPortsTtl = "";
    int portIndex = 0;

    for (int i = 0; i < project.numInputs; ++i) {
        audioPortsTtl += "[ a lv2:InputPort, lv2:AudioPort ; lv2:index " + juce::String(portIndex) +
                         " ; lv2:symbol \\\"in_" + juce::String(i) + "\\\" ; lv2:name \\\"In " + juce::String(i+1) + "\\\" ]";
        portIndex++;
        if (i < project.numInputs - 1 || project.numOutputs > 0) audioPortsTtl += ",\n             ";
    }

    for (int i = 0; i < project.numOutputs; ++i) {
        audioPortsTtl += "[ a lv2:OutputPort, lv2:AudioPort ; lv2:index " + juce::String(portIndex) +
                         " ; lv2:symbol \\\"out_" + juce::String(i) + "\\\" ; lv2:name \\\"Out " + juce::String(i+1) + "\\\" ]";
        portIndex++;
        if (i < project.numOutputs - 1) audioPortsTtl += ",\n             ";
    }

    // ================================
    // 2. CONTROL PORTS
    // ================================
    juce::String ttlPorts = "";

    for (const auto& comp : project.components)
    {
        ttlPorts += ",\n             [ ";

        if (comp.type == PluginData::ComponentType::Slider || comp.type == PluginData::ComponentType::Knob)
        {
            ttlPorts += "a lv2:InputPort, lv2:ControlPort ; ";
            ttlPorts += "lv2:index " + juce::String(portIndex) + " ; ";
            ttlPorts += "lv2:symbol \\\"" + comp.symbol + "\\\" ; ";
            ttlPorts += "lv2:name \\\"" + comp.name + "\\\" ; ";
            ttlPorts += "lv2:default " + juce::String(comp.def) + " ; ";
            ttlPorts += "lv2:minimum " + juce::String(comp.min) + " ; ";
            ttlPorts += "lv2:maximum " + juce::String(comp.max);
        }
        else if (comp.type == PluginData::ComponentType::Toggle)
        {
            ttlPorts += "a lv2:InputPort, lv2:ControlPort ; ";
            ttlPorts += "lv2:portProperty lv2:toggled ; ";
            ttlPorts += "lv2:index " + juce::String(portIndex) + " ; ";
            ttlPorts += "lv2:symbol \\\"" + comp.symbol + "\\\" ; ";
            ttlPorts += "lv2:name \\\"" + comp.name + "\\\" ; ";
            ttlPorts += "lv2:default " + juce::String(comp.def) + " ; ";
            ttlPorts += "lv2:minimum 0 ; lv2:maximum 1";
        }

        ttlPorts += " ] ";
        portIndex++;
    }

    // ================================
    // 3. MAPEO POR ROLE 
    // ================================

    int driveParam = -1;
    int mixParam   = -1;
    int toneParam  = -1;
    int gainParam  = -1;
    int freqParam  = -1;
    int depthParam = -1;
    int toggleParam = -1;

    for (int i = 0; i < project.components.size(); ++i)
    {
        const auto& comp = project.components[i];

        if (comp.role == PluginData::ParamRole::Drive) driveParam = i;
        else if (comp.role == PluginData::ParamRole::Mix) mixParam = i;
        else if (comp.role == PluginData::ParamRole::Tone) toneParam = i;
        else if (comp.role == PluginData::ParamRole::Gain) gainParam = i;
        else if (comp.role == PluginData::ParamRole::Frequency) freqParam = i;
        else if (comp.role == PluginData::ParamRole::Depth) depthParam = i;

        if (comp.type == PluginData::ComponentType::Toggle && toggleParam == -1)
            toggleParam = i;
    }

    auto getParam = [&](int idx, juce::String def)
    {
        return (idx >= 0) ? "params[" + juce::String(idx) + "]" : def;
    };

    juce::String driveStr = getParam(driveParam, "0.5f");
    juce::String mixStr   = getParam(mixParam, "1.0f");
    juce::String toneStr  = getParam(toneParam, "1.0f");
    juce::String gainStr  = getParam(gainParam, "1.0f");
    juce::String freqStr  = getParam(freqParam, "1.0f");
    juce::String depthStr = getParam(depthParam, "0.5f");
    juce::String toggleStr = getParam(toggleParam, "1.0f");

    juce::String dspCode;

    // ================================
    // 4. DSP GENERATION
    // ================================

    switch (project.currentAlgorithm)
    {
        case PluginData::AlgorithmType::Distortion:
            dspCode = R"(

            float drive = 1.0f + ()" + driveStr + R"( * 10.0f);
            float mix = )" + mixStr + R"(;
            float tone = )" + toneStr + R"(;
            bool enabled = ()" + toggleStr + R"( > 0.5f);

            for (int ch = 0; ch < totalNumInputChannels; ++ch)
            {
                auto* data = buffer.getWritePointer(ch);

                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    float clean = data[i];

                    if (enabled)
                    {
                        float distorted = std::tanh(clean * drive);
                        float out = (clean * (1.0f - mix)) + (distorted * mix);
                        out *= tone;
                        data[i] = out;
                    }
                }
            }
            )";
            break;

        case PluginData::AlgorithmType::Tremolo:
            dspCode = R"(

            float freq = )" + freqStr + R"(;
            float depth = )" + depthStr + R"(;
            bool enabled = ()" + toggleStr + R"( > 0.5f);

            static float phase = 0.0f;
            float sr = getSampleRate();
            float inc = (freq * 2.0f * juce::MathConstants<float>::pi) / sr;

            for (int ch = 0; ch < totalNumInputChannels; ++ch)
            {
                auto* data = buffer.getWritePointer(ch);

                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    if (enabled)
                    {
                        float lfo = 1.0f - (depth * 0.5f * (1.0f + std::sin(phase)));
                        data[i] *= lfo;
                    }

                    phase += inc;
                    if (phase > 2.0f * juce::MathConstants<float>::pi)
                        phase -= 2.0f * juce::MathConstants<float>::pi;
                }
            }
            )";
            break;

        case PluginData::AlgorithmType::Gain:
        default:
            dspCode = R"(

            float gain = )" + gainStr + R"(;
            bool enabled = ()" + toggleStr + R"( > 0.5f);

            for (int ch = 0; ch < totalNumInputChannels; ++ch)
            {
                auto* data = buffer.getWritePointer(ch);

                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    if (enabled)
                        data[i] *= gain;
                }
            }
            )";
            break;

        case PluginData::AlgorithmType::Custom:
            dspCode = project.customDspCode;
            break;
    }

    // ================================
    // 5. FILE GENERATION
    // ================================
    juce::String cmakeContent = Templates::cmakeFile;

    cmakeContent = cmakeContent.replace("{{AUDIO_PORTS_TTL}}", audioPortsTtl);
    cmakeContent = cmakeContent.replace("{{TTL_PORTS}}", ttlPorts);
    cmakeContent = cmakeContent.replace("{{PLUGIN_NAME}}", project.pluginName);
    cmakeContent = cmakeContent.replace("{{PLUGIN_URI}}", project.pluginURI);

    juce::String editorContent = Templates::editorCpp;
    editorContent = editorContent.replace("{{NUM_INPUTS}}", juce::String(project.numInputs));
    editorContent = editorContent.replace("{{NUM_OUTPUTS}}", juce::String(project.numOutputs));
    editorContent = editorContent.replace("{{NUM_PARAMS}}", juce::String(project.components.size()));

    juce::String processorContent = Templates::processorCpp;
    if (processorContent.contains("{{DSP_CODE}}"))
        processorContent = processorContent.replace("{{DSP_CODE}}", dspCode);
    else
        processorContent = processorContent.replace("// INSERT_DSP_HERE", dspCode);

    // INIT
    if (processorContent.contains("{{INIT_CODE}}"))
        processorContent = processorContent.replace("{{INIT_CODE}}", project.initCode);

    desktopDir.getChildFile("CMakeLists.txt").replaceWithText(cmakeContent);
    sourceDir.getChildFile("PluginProcessor.h").replaceWithText(Templates::processorHeader);
    sourceDir.getChildFile("PluginProcessor.cpp").replaceWithText(processorContent);
    sourceDir.getChildFile("PluginEditor.h").replaceWithText(Templates::editorHeader);
    sourceDir.getChildFile("PluginEditor.cpp").replaceWithText(editorContent);
}

// ==============================================================================
// COMPILAR E INSTALAR
// ==============================================================================
juce::String PluginGenerator::compileAndInstallPlugin(const PluginData::Project& project)
{
    juce::String safeName = "";
    for (auto c : project.pluginName)
        if (juce::CharacterFunctions::isLetter(c))
            safeName += c;

    if (safeName.isEmpty()) safeName = "MiEfectoDSP";

    juce::String cmd =
        "cd \"" + desktopDir.getFullPathName() + "\" && "
        "mkdir -p build && cd build && "
        "cmake .. && cmake --build . -j4 && "
        "rm -rf ~/.lv2/" + safeName + ".lv2 && "
        "mkdir -p ~/.lv2/" + safeName + ".lv2 && "
        "cp MiEfectoDSP.so manifest.ttl plugin.ttl ~/.lv2/" + safeName + ".lv2/";

    return (system(cmd.toRawUTF8()) == 0)
        ? "OK:" + safeName
        : "ERROR: fallo en compilación";
}