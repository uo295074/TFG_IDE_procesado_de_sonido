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

    // --- 1. PROCESADOR (Lo que ya tenías) ---
    std::ofstream headerFile(sourceDir / "PluginProcessor.h");
    if (headerFile.is_open()) { headerFile << Templates::processorHeader; headerFile.close(); }

    std::string cppContent = Templates::processorCpp;
    std::string tag = "// *** USER_CODE_TAG ***";
    size_t pos = cppContent.find(tag);
    if (pos != std::string::npos)
        cppContent.replace(pos, tag.length(), customCode);

    std::ofstream cppFile(sourceDir / "PluginProcessor.cpp");
    if (cppFile.is_open()) { cppFile << cppContent; cppFile.close(); }


    // --- 2. EDITOR (NUEVO) ---
    // Escribimos PluginEditor.h
    std::ofstream editorHeaderFile(sourceDir / "PluginEditor.h");
    if (editorHeaderFile.is_open()) { 
        editorHeaderFile << Templates::editorHeader; 
        editorHeaderFile.close(); 
    }

    // Escribimos PluginEditor.cpp
    std::ofstream editorCppFile(sourceDir / "PluginEditor.cpp");
    if (editorCppFile.is_open()) { 
        editorCppFile << Templates::editorCpp; 
        editorCppFile.close(); 
    }


    // --- 3. CMAKE (NUEVO) ---
    // Este va en la raíz del proyecto, no en Source
    std::ofstream cmakeFile(p / "CMakeLists.txt");
    if (cmakeFile.is_open()) { 
        cmakeFile << Templates::cmakeFile; 
        cmakeFile.close(); 
    }

    return true;
}

/**
Este archivo define "cómo funciona". 
Aquí usamos std::ofstream (output file stream) para crear archivos reales. */