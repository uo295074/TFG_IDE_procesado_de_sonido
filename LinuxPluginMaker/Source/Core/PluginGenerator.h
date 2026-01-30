/*
  ==============================================================================
    Ruta: Source/Core/PluginGenerator.h
  ==============================================================================
*/

#pragma once
#include <string>
#include <vector>

class PluginGenerator
{
public:
    PluginGenerator();
    ~PluginGenerator();

    // Setters para los datos del usuario
    void setProjectName (std::string name);
    void setCustomCode (std::string code); // <--- NUEVO: Para el código C++ del usuario
    void setIOSettings (int inputs, int outputs); // <--- NUEVO: Configuración de canales

    // El disparador principal
    bool generateProject (std::string destinationPath);

private:
    std::string projectName;
    std::string customCode; 
    int numInputs = 2;
    int numOutputs = 2;

    // Métodos internos
    bool createDirectoryStructure (std::string projectPath);
    bool createPluginFiles (std::string projectPath); 
};

/**
Este archivo define "qué variables tenemos". 
Hemos añadido customCode y configuración de canales. */