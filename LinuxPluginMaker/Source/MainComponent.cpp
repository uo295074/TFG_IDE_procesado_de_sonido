/*
  ==============================================================================
    Ruta: Source/MainComponent.cpp
  ==============================================================================
*/

#include "MainComponent.h"

MainComponent::MainComponent()
{
    // 1. Nombre del Plugin
    nameLabel.setText ("Nombre del Plugin:", juce::dontSendNotification);
    addAndMakeVisible (nameLabel);

    nameEditor.setText ("MiEfectoDSP");
    addAndMakeVisible (nameEditor);

    // 2. Editor de Código (NUEVO)
    codeLabel.setText ("Tu Algoritmo (C++):", juce::dontSendNotification);
    addAndMakeVisible (codeLabel);

    codeEditor.setMultiLine (true); // Permitir varias líneas
    codeEditor.setReturnKeyStartsNewLine (true); // Enter baja de línea
    codeEditor.setText (R"(// Escribe aquí tu código DSP...
// Ejemplo: Reducir ganancia al 50%

for (int channel = 0; channel < numChannels; ++channel)
{
    auto* channelData = buffer.getWritePointer (channel);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // channelData[sample] es la muestra actual
        channelData[sample] *= 0.5f; 
    }
})");
    addAndMakeVisible (codeEditor);

    // 3. Botón Generar
    generateButton.setButtonText ("Generar Proyecto");
    generateButton.onClick = [this] { generatePlugin(); };
    addAndMakeVisible (generateButton);

    setSize (500, 600); // Ventana más alta para que quepa el código
}

MainComponent::~MainComponent() {}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (20);

    // Fila 1: Nombre
    auto rowName = area.removeFromTop (30);
    nameLabel.setBounds (rowName.removeFromLeft (120));
    nameEditor.setBounds (rowName);

    area.removeFromTop (20); // Espacio

    // Fila 2: Etiqueta Código
    codeLabel.setBounds (area.removeFromTop (30));

    // Botón abajo del todo (40px de alto)
    generateButton.setBounds (area.removeFromBottom (40));

    area.removeFromBottom (20); // Espacio antes del botón

    // El resto del espacio para el editor de código
    codeEditor.setBounds (area);
}

void MainComponent::generatePlugin()
{
    // 1. Recoger datos
    juce::String name = nameEditor.getText();
    juce::String code = codeEditor.getText();

    if (name.isEmpty()) return;

    // 2. Pasar datos al generador
    generator.setProjectName (name.toStdString());
    generator.setCustomCode (code.toStdString());
    generator.setIOSettings (2, 2); // Por defecto Estéreo

    // 3. Ejecutar
    juce::File desktop = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);
    bool success = generator.generateProject (desktop.getFullPathName().toStdString());

    // 4. Feedback visual simple
    if (success)
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Genial", "Código guardado en el Escritorio.");
    else
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Error", "Fallo al escribir el archivo.");
}
