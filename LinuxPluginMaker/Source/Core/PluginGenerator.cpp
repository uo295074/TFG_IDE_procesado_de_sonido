/*
  ==============================================================================
    Ruta: Source/Core/PluginGenerator.cpp
  ==============================================================================
*/

#include "PluginGenerator.h"
#include <filesystem>
#include <fstream> // Necesario para escribir dentro de archivos
#include <iostream>

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
        return writeTestFile(projectPath.string());
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

bool PluginGenerator::writeTestFile (std::string pathStr)
{
    fs::path p(pathStr);
    // Creamos un archivo llamado "UserCode.cpp" dentro de la carpeta Source
    fs::path filePath = p / "Source" / "UserCode.cpp";

    std::ofstream outfile (filePath);
    
    if (!outfile.is_open()) return false;

    // Escribimos una cabecera y el código del usuario
    outfile << "// Archivo generado automáticamente por LinuxPluginMaker\n";
    outfile << "// Configuración I/O: " << numInputs << " In / " << numOutputs << " Out\n\n";
    outfile << "void processBlock (float* buffer, int numSamples)\n{\n";
    outfile << customCode << "\n"; // <--- Aquí inyectamos lo que escribió el usuario
    outfile << "}\n";

    outfile.close();
    return true;
}

/**
Este archivo define "cómo funciona". 
Aquí usamos std::ofstream (output file stream) para crear archivos reales. */