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
    // 1. Crear carpeta si no existe
    if (!desktopDir.exists())
        desktopDir.createDirectory();

    juce::File sourceDir = desktopDir.getChildFile("Source");
    if (!sourceDir.exists())
        sourceDir.createDirectory();

    // 2. GENERAR CONTENIDO TTL DINÁMICO
    juce::String ttlPorts = "";
    
    for (const auto& comp : project.components)
    {
        // La coma inicial para separar del puerto anterior
        ttlPorts += ",\n             [ "; 
        
        if (comp.type == PluginData::ComponentType::Slider)
        {
            ttlPorts += "a lv2:InputPort, lv2:ControlPort ; ";
            ttlPorts += "lv2:index " + juce::String(comp.index) + " ; ";
            // AQUÍ EL FIX DE LAS COMILLAS:
            ttlPorts += "lv2:symbol \"" + comp.symbol + "\" ; "; 
            ttlPorts += "lv2:name \"" + comp.name + "\" ; ";
            ttlPorts += "lv2:default " + juce::String(comp.def) + " ; ";
            ttlPorts += "lv2:minimum " + juce::String(comp.min) + " ; ";
            // AQUÍ EL FIX DEL PUNTO Y COMA (lo quitamos al final):
            ttlPorts += "lv2:maximum " + juce::String(comp.max); 
        }
        else if (comp.type == PluginData::ComponentType::Toggle)
        {
            ttlPorts += "a lv2:InputPort, lv2:ControlPort, lv2:toggled ; ";
            ttlPorts += "lv2:index " + juce::String(comp.index) + " ; ";
            ttlPorts += "lv2:symbol \"" + comp.symbol + "\" ; ";
            ttlPorts += "lv2:name \"" + comp.name + "\" ; ";
            ttlPorts += "lv2:default " + juce::String(comp.def) + " ; ";
            ttlPorts += "lv2:minimum 0 ; lv2:maximum 1"; // Sin punto y coma final
        }

        ttlPorts += " ] "; // Cerramos el corchete limpiamente
    }
    // 3. PREPARAR ARCHIVOS (Sustitución de etiquetas)
    
    // -> CMakeLists.txt
    juce::String cmakeContent = Templates::cmakeFile;
    cmakeContent = cmakeContent.replace("{{TTL_PORTS}}", ttlPorts);
    
    // -> PluginEditor.cpp (Wrapper)
    juce::String editorContent = Templates::editorCpp;
    // Sustituimos {{NUM_PARAMS}} por el número real
    editorContent = editorContent.replace("{{NUM_PARAMS}}", juce::String(project.components.size()));

    // 4. ESCRIBIR EN DISCO
    desktopDir.getChildFile("CMakeLists.txt").replaceWithText(cmakeContent);
    sourceDir.getChildFile("PluginProcessor.h").replaceWithText(Templates::processorHeader);
    sourceDir.getChildFile("PluginProcessor.cpp").replaceWithText(Templates::processorCpp);
    sourceDir.getChildFile("PluginEditor.h").replaceWithText(Templates::editorHeader);
    sourceDir.getChildFile("PluginEditor.cpp").replaceWithText(editorContent);

    // Aviso en consola (o AlertWindow si estuviéramos en MainComponent)
    juce::Logger::writeToLog("Archivos generados en: " + desktopDir.getFullPathName());
}