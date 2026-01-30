/*
  ==============================================================================
    Ruta: Source/Core/PluginGenerator.cpp
  ==============================================================================
*/

#include "PluginGenerator.h"
#include <filesystem>
#include <fstream> // Necesario para escribir dentro de archivos
#include <iostream>
#include "Templates.h"

namespace fs = std::filesystem;

PluginGenerator::PluginGenerator() {}
PluginGenerator::~PluginGenerator() {}

void PluginGenerator::setProjectName (std::string name) { projectName = name; }
void PluginGenerator::setCustomCode (std::string code)  { customCode = code; }
void PluginGenerator::setIOSettings (int inputs, int outputs) 
{ 
    numInputs = inputs; 
    numOutputs = outputs; 
}

bool PluginGenerator::generateProject (std::string destinationPath)
{
    // Construimos la ruta segura
    fs::path basePath(destinationPath);
    fs::path projectPath = basePath / projectName;

    try
    {
        // 1. Crear carpeta principal
        if (fs::exists(projectPath))
        {
            std::cout << "Aviso: La carpeta ya existe, se sobreescribirá." << std::endl;
        }
        fs::create_directories(projectPath);

        // 2. Crear subcarpetas
        createDirectoryStructure(projectPath.string());

        // 3. Escribir el archivo con el código del usuario (Prueba)
        return createPluginFiles(projectPath.string());
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
        return false;
    }
}

bool PluginGenerator::createDirectoryStructure (std::string pathStr)
{
    fs::path p(pathStr);
    fs::create_directories(p / "Source");
    return true;
}

bool PluginGenerator::createPluginFiles(std::string projectPath)
{
    fs::path p(projectPath);
    fs::path sourceDir = p / "Source";

    // 1. Escribir el Header (.h) tal cual, sin cambios
    std::ofstream headerFile(sourceDir / "PluginProcessor.h");
    if (!headerFile.is_open()) return false;
    
    headerFile << Templates::processorHeader;
    headerFile.close();

    // 2. Preparar el Cpp (.cpp) con el código del usuario
    std::string cppContent = Templates::processorCpp;
    
    // Buscamos la etiqueta mágica "*** USER_CODE_TAG ***"
    std::string tag = "// *** USER_CODE_TAG ***";
    size_t pos = cppContent.find(tag);

    if (pos != std::string::npos)
    {
        // Reemplazamos la etiqueta por el código que escribió Marcos en la ventana
        cppContent.replace(pos, tag.length(), customCode);
    }

    // 3. Escribir el archivo Cpp final
    std::ofstream cppFile(sourceDir / "PluginProcessor.cpp");
    if (!cppFile.is_open()) return false;

    cppFile << cppContent;
    cppFile.close();

    return true;
}

/**
Este archivo define "cómo funciona". 
Aquí usamos std::ofstream (output file stream) para crear archivos reales. */