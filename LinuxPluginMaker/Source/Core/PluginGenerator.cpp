/*
  ==============================================================================
    Source/Core/PluginGenerator.cpp
    Actualizado para inyectar configuración de Canales de Audio (Mono/Estéreo)
  ==============================================================================
*/

#include "PluginGenerator.h"
#include "Templates.h"

PluginGenerator::PluginGenerator()
{
    // Apuntamos al Escritorio/MiEfectoDSP
    desktopDir = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                 .getChildFile("MiEfectoDSP");
}

void PluginGenerator::createPluginFiles(PluginData::Project& project)
{
    if (!desktopDir.exists()) desktopDir.createDirectory();
    juce::File sourceDir = desktopDir.getChildFile("Source");
    if (!sourceDir.exists()) sourceDir.createDirectory();

    // --- 1. GENERAR PUERTOS DE AUDIO (DINÁMICO MONO/ESTÉREO) ---
    juce::String audioPortsTtl = "";
    int portIndex = 0;
    
    // Entradas
    for (int i = 0; i < project.numInputs; ++i) {
        audioPortsTtl += "[ a lv2:InputPort, lv2:AudioPort ; lv2:index " + juce::String(portIndex) + 
                         " ; lv2:symbol \\\"in_" + juce::String(i) + "\\\" ; lv2:name \\\"In " + juce::String(i+1) + "\\\" ]";
        portIndex++;
        // Ponemos coma si no es el último puerto en total
        if (i < project.numInputs - 1 || project.numOutputs > 0) audioPortsTtl += ",\n             ";
    }
    
    // Salidas
    for (int i = 0; i < project.numOutputs; ++i) {
        audioPortsTtl += "[ a lv2:OutputPort, lv2:AudioPort ; lv2:index " + juce::String(portIndex) + 
                         " ; lv2:symbol \\\"out_" + juce::String(i) + "\\\" ; lv2:name \\\"Out " + juce::String(i+1) + "\\\" ]";
        portIndex++;
        if (i < project.numOutputs - 1) audioPortsTtl += ",\n             ";
    }

    // --- 2. GENERAR PUERTOS DE PARÁMETROS ---
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

    // 3. GENERAR CÓDIGO DSP MATEMÁTICO
    juce::String dspCode = "";

    switch (project.currentAlgorithm)
    {
        case PluginData::AlgorithmType::Distortion:
            dspCode = R"(
                // --- CÓDIGO GENERADO: DISTORSIÓN ---
                float drive = 1.0f;
                if (params.size() > 0) drive = params[0]; 

                drive = 1.0f + (drive * 10.0f); 

                for (int channel = 0; channel < totalNumInputChannels; ++channel)
                {
                    auto* channelData = buffer.getWritePointer (channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                    {
                        channelData[sample] = std::tanh(channelData[sample] * drive);
                    }
                }
            )";
            break;

        case PluginData::AlgorithmType::Tremolo:
            dspCode = R"(
                // --- CÓDIGO GENERADO: TRÉMOLO ---
                float freq = 1.0f; 
                float depth = 0.5f;
                
                if (params.size() > 0) freq = params[0];
                if (params.size() > 1) depth = params[1];

                static float currentPhase = 0.0f; 
                float sampleRate = getSampleRate();
                float phaseIncrement = (freq * 2.0f * juce::MathConstants<float>::pi) / sampleRate;

                for (int channel = 0; channel < totalNumInputChannels; ++channel)
                {
                    auto* channelData = buffer.getWritePointer (channel);
                    float phase = currentPhase; 
                    
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                    {
                        float lfo = 1.0f - (depth * 0.5f * (1.0f + std::sin(phase)));
                        channelData[sample] *= lfo;
                        
                        phase += phaseIncrement;
                        if (phase >= 2.0f * juce::MathConstants<float>::pi) phase -= 2.0f * juce::MathConstants<float>::pi;
                    }
                    if (channel == totalNumInputChannels - 1) currentPhase = phase; 
                }
            )";
            break;

        case PluginData::AlgorithmType::Gain:
        default:
            dspCode = R"(
                // --- CÓDIGO GENERADO: GANANCIA ---
                float gain = 1.0f;
                if (params.size() > 0) gain = params[0]; 

                for (int channel = 0; channel < totalNumInputChannels; ++channel)
                {
                    auto* channelData = buffer.getWritePointer (channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                    {
                        channelData[sample] *= gain;
                    }
                }
            )";
            break;
            
        case PluginData::AlgorithmType::Custom:
            // --- CÓDIGO GENERADO: PERSONALIZADO POR EL USUARIO ---
            // Simplemente pegamos lo que el técnico escribió en la ventana
            dspCode = project.customDspCode;
            break;
    }

    // 4. PREPARAR ARCHIVOS
    juce::String cmakeContent = Templates::cmakeFile;
    
    juce::String cleanURI = project.pluginURI.replace("\r", "").replace("\n", "").trim();
    juce::String cleanName = project.pluginName.replace("\r", "").replace("\n", "").trim();

    cmakeContent = cmakeContent.replace("{{AUDIO_PORTS_TTL}}", audioPortsTtl);
    cmakeContent = cmakeContent.replace("{{TTL_PORTS}}", ttlPorts);
    cmakeContent = cmakeContent.replace("{{PLUGIN_NAME}}", cleanName); 
    cmakeContent = cmakeContent.replace("{{PLUGIN_URI}}", cleanURI);  
    
    juce::String editorContent = Templates::editorCpp;
    editorContent = editorContent.replace("{{NUM_INPUTS}}", juce::String(project.numInputs));
    editorContent = editorContent.replace("{{NUM_OUTPUTS}}", juce::String(project.numOutputs));
    editorContent = editorContent.replace("{{NUM_PARAMS}}", juce::String(project.components.size()));

    juce::String processorContent = Templates::processorCpp;
    
    if (processorContent.contains("{{DSP_CODE}}"))
        processorContent = processorContent.replace("{{DSP_CODE}}", dspCode);
    else
        processorContent = processorContent.replace("// INSERT_DSP_HERE", dspCode);

    // 5. ESCRIBIR EN DISCO
    desktopDir.getChildFile("CMakeLists.txt").replaceWithText(cmakeContent);
    sourceDir.getChildFile("PluginProcessor.h").replaceWithText(Templates::processorHeader);
    sourceDir.getChildFile("PluginProcessor.cpp").replaceWithText(processorContent);
    sourceDir.getChildFile("PluginEditor.h").replaceWithText(Templates::editorHeader);
    sourceDir.getChildFile("PluginEditor.cpp").replaceWithText(editorContent);

    juce::Logger::writeToLog("Archivos generados correctamente en: " + desktopDir.getFullPathName());
}

// ==============================================================================
// AUTOMATIZACIÓN DE COMPILACIÓN E INSTALACIÓN
// ==============================================================================
juce::String PluginGenerator::compileAndInstallPlugin(const PluginData::Project& project)
{
    juce::String safeName = "";
    for (auto c : project.pluginName) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            safeName += c;
        }
    }
    if (safeName.isEmpty()) safeName = "MiEfectoDSP";

    juce::String bashCommands = 
        "cd \"" + desktopDir.getFullPathName() + "\" && "
        "mkdir -p build && "
        "cd build && "
        "cmake .. && "
        "cmake --build . -j4 && "
        "rm -rf ~/.lv2/" + safeName + ".lv2 && "
        "mkdir -p ~/.lv2/" + safeName + ".lv2 && "
        "cp MiEfectoDSP.so manifest.ttl plugin.ttl ~/.lv2/" + safeName + ".lv2/";

    int result = system(bashCommands.toRawUTF8());

    if (result == 0) {
        return "OK:" + safeName;
    } else {
        return "ERROR: Revisa la terminal donde lanzaste el IDE para ver qué falló.";
    }
}