/*
  ==============================================================================
    Source/MainComponent.cpp
  ==============================================================================
*/

#include "MainComponent.h"

MainComponent::MainComponent()
{
    // 1. Configurar Títulos
    addAndMakeVisible(toolsLabel);
    toolsLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    toolsLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(listLabel);
    listLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    listLabel.setJustificationType(juce::Justification::centred);

    // 2. Configurar Botones de Herramientas
    addAndMakeVisible(addSliderBtn);
    addSliderBtn.onClick = [this]
    {
        // Añadimos un Slider Genérico (Luego haremos que pida nombre)
        int id = project.components.size() + 1;
        project.addSlider("Control " + juce::String(id), 0.0f, 1.0f, 0.5f);
        updateListView();
    };

    addAndMakeVisible(addToggleBtn);
    addToggleBtn.onClick = [this]
    {
        int id = project.components.size() + 1;
        project.addToggle("Interruptor " + juce::String(id));
        updateListView();
    };

    addAndMakeVisible(clearBtn);
    clearBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    clearBtn.onClick = [this]
    {
        project.components.clear();
        updateListView();
    };

    // 3. Configurar Lista Visual (Log)
    addAndMakeVisible(componentsLog);
    componentsLog.setMultiLine(true);
    componentsLog.setReadOnly(true);
    componentsLog.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    componentsLog.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
    componentsLog.setFont(juce::Font("Consolas", 14.0f, juce::Font::plain));
    componentsLog.setText("Proyecto vacío. Añade componentes...");

    // 4. Configurar Botón Generar
    addAndMakeVisible(generateBtn);
    generateBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    generateBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    generateBtn.onClick = [this]
    {
        // AQUÍ CONECTAREMOS CON EL GENERADOR REAL MÁS ADELANTE
        // Por ahora simulamos
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
            "Generando...", 
            "Se ha enviado el proyecto con " + juce::String(project.components.size()) + " componentes.");
            
        generator.createPluginFiles(project); // <-- Esto lo activaremos luego
    };

    setSize(800, 600);
}

MainComponent::~MainComponent() {}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey); // Fondo general

    // Dibujar separación entre zona herramientas y lista
    g.setColour(juce::Colours::black);
    g.fillRect(200, 0, 2, getHeight()); 
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // --- COLUMNA IZQUIERDA (200px) ---
    auto sidebar = area.removeFromLeft(200);
    sidebar.reduce(10, 10); // Márgenes

    toolsLabel.setBounds(sidebar.removeFromTop(30));
    sidebar.removeFromTop(20); // Espacio

    addSliderBtn.setBounds(sidebar.removeFromTop(40));
    sidebar.removeFromTop(10);
    addToggleBtn.setBounds(sidebar.removeFromTop(40));
    
    // Botón borrar abajo del todo de la barra lateral
    clearBtn.setBounds(sidebar.removeFromBottom(40));


    // --- ZONA CENTRAL (El resto) ---
    area.reduce(20, 20); // Márgenes
    
    listLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);
    
    // El botón generar abajo del todo
    generateBtn.setBounds(area.removeFromBottom(50));
    area.removeFromBottom(20);

    // La lista ocupa todo el espacio central restante
    componentsLog.setBounds(area);
}

void MainComponent::updateListView()
{
    juce::String text = "--- Componentes del Plugin ---\n\n";

    for (const auto& c : project.components)
    {
        juce::String tipo = (c.type == PluginData::ComponentType::Slider) ? "[SLIDER]" : "[SWITCH]";
        text += tipo + "  " + c.name + "   (ID: " + c.symbol + ", Puerto: " + juce::String(c.index) + ")\n";
    }

    componentsLog.setText(text);
}