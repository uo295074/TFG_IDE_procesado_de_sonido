/*
  ==============================================================================
    PluginGenerator.h
    Descripción: El cerebro del IDE. Se encarga de orquestar la creación de archivos.
  ==============================================================================
*/

#pragma once
#include <string>
#include <vector>

// Usamos un enum class para definir tipos fuertemente tipados (Buenas prácticas C++)
enum class PluginType {
    Effect,
    Instrument
};

class PluginGenerator
{
public:
    PluginGenerator();
    ~PluginGenerator();

    // Establecer datos básicos
    void setProjectName (std::string name);
    void setPluginCode (std::string code);
    
    // El método principal que "apretará el botón"
    bool generateProject (std::string destinationPath);

private:
    std::string projectName;
    std::string pluginCode;
    PluginType currentType = PluginType::Effect;
    
    // Método interno para crear la estructura de carpetas
    bool createDirectoryStructure();
};