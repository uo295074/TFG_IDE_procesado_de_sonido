/*
  ==============================================================================
    Source/Core/PluginGenerator.cpp
    (CORREGIDO: Eliminados los asteriscos '*' sobrantes en params)
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

    juce::String ttlPorts = "";
    int portIndex = 4; 
    
    for (const auto& comp : project.components)
    {
        ttlPorts += ",\n             [ "; 
        
        if (comp.type == PluginData::ComponentType::Slider)
        {
            ttlPorts += "a lv2:InputPort, lv2:ControlPort ; ";
            ttlPorts += "lv2:index " + juce::String(portIndex) + " ; "; 
            
            // --- AQUÍ ESTÁ EL ARREGLO DE LAS COMILLAS (\\\" en vez de \") ---
            ttlPorts += "lv2:symbol \\\"" + comp.symbol + "\\\" ; "; 
            ttlPorts += "lv2:name \\\"" + comp.name + "\\\" ; ";
            
            ttlPorts += "lv2:default " + juce::String(comp.def) + " ; ";
            ttlPorts += "lv2:minimum " + juce::String(comp.min) + " ; ";
            ttlPorts += "lv2:maximum " + juce::String(comp.max); 
        }
        else if (comp.type == PluginData::ComponentType::Toggle)
        {
            ttlPorts += "a lv2:InputPort, lv2:ControlPort ; ";
            // --- ESTA ES LA LÍNEA QUE FALTABA (La propiedad correcta) ---
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

    // 3. GENERAR CÓDIGO DSP MATEMÁTICO (CORREGIDO SIN ASTERISCOS)
    juce::String dspCode = "";

    switch (project.currentAlgorithm)
    {
        case PluginData::AlgorithmType::Distortion:
            // Algoritmo de Distorsión
            dspCode = R"(
                // --- CÓDIGO GENERADO: DISTORSIÓN ---
                float drive = 1.0f;
                // CORRECCIÓN: Quitamos el '*' porque params[0] ya es un float
                if (params.size() > 0) drive = params[0]; 

                // Evitar silencio si el drive es 0, sumamos 1
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
            // Algoritmo de Trémolo
            dspCode = R"(
                // --- CÓDIGO GENERADO: TRÉMOLO ---
                float freq = 1.0f; 
                float depth = 0.5f;
                
                // CORRECCIÓN: Quitamos los '*'
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
            // Algoritmo Ganancia
            dspCode = R"(
                // --- CÓDIGO GENERADO: GANANCIA ---
                float gain = 1.0f;
                // CORRECCIÓN: Quitamos el '*'
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
    }

    // 4. PREPARAR ARCHIVOS
    juce::String cmakeContent = Templates::cmakeFile;
    
    // Limpiamos el URI de saltos de línea invisibles por si acaso
    juce::String cleanURI = project.pluginURI.replace("\r", "").replace("\n", "").trim();
    juce::String cleanName = project.pluginName.replace("\r", "").replace("\n", "").trim();

    cmakeContent = cmakeContent.replace("{{TTL_PORTS}}", ttlPorts);
    cmakeContent = cmakeContent.replace("{{PLUGIN_NAME}}", cleanName); 
    cmakeContent = cmakeContent.replace("{{PLUGIN_URI}}", cleanURI);  
    
    juce::String editorContent = Templates::editorCpp;
    editorContent = editorContent.replace("{{NUM_PARAMS}}", juce::String(project.components.size()));

    juce::String processorContent = Templates::processorCpp;
    if (processorContent.contains("{{DSP_CODE}}"))
    {
        processorContent = processorContent.replace("{{DSP_CODE}}", dspCode);
    }
    else
    {
        processorContent = processorContent.replace("// INSERT_DSP_HERE", dspCode);
    }

    // 5. ESCRIBIR EN DISCO
    desktopDir.getChildFile("CMakeLists.txt").replaceWithText(cmakeContent);
    sourceDir.getChildFile("PluginProcessor.h").replaceWithText(Templates::processorHeader);
    sourceDir.getChildFile("PluginProcessor.cpp").replaceWithText(processorContent);
    sourceDir.getChildFile("PluginEditor.h").replaceWithText(Templates::editorHeader);
    sourceDir.getChildFile("PluginEditor.cpp").replaceWithText(editorContent);

    juce::Logger::writeToLog("Archivos generados correctamente en: " + desktopDir.getFullPathName());
}

// ==============================================================================
// RU-03: AUTOMATIZACIÓN DE COMPILACIÓN E INSTALACIÓN
// ==============================================================================
juce::String PluginGenerator::compileAndInstallPlugin(const PluginData::Project& project)
{
    // 1. Nombre hiper-seguro (solo letras, sin espacios ni símbolos raros)
    juce::String safeName = "";
    for (auto c : project.pluginName) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            safeName += c;
        }
    }
    if (safeName.isEmpty()) safeName = "MiEfectoDSP";

    // 2. Los comandos justos y necesarios
    juce::String bashCommands = 
        "cd \"" + desktopDir.getFullPathName() + "\" && "
        "mkdir -p build && "
        "cd build && "
        "cmake .. && "
        "cmake --build . -j4 && "
        "rm -rf ~/.lv2/" + safeName + ".lv2 && "
        "mkdir -p ~/.lv2/" + safeName + ".lv2 && "
        "cp MiEfectoDSP.so manifest.ttl plugin.ttl ~/.lv2/" + safeName + ".lv2/";

    // 3. Ejecución directa del sistema (bloqueará la UI 3 segundos, pero NO se colgará)
    int result = system(bashCommands.toRawUTF8());

    // Si system() devuelve 0, significa que todo salió perfecto
    if (result == 0) {
        return "OK:" + safeName;
    } else {
        return "ERROR: Revisa la terminal donde lanzaste el IDE para ver qué falló.";
    }
}