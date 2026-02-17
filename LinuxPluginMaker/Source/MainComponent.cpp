/*
  ==============================================================================
    Source/MainComponent.cpp
  ==============================================================================
*/
#include "MainComponent.h"

// Definimos IDs para los menús para no liarnos con números sueltos
enum MenuIDs {
    FileNew = 1, FileLoad, FileSave, FileExit,
    ProjectProps, ProjectGenerate,
    HelpAbout
};

MainComponent::MainComponent()
    : menuBar(this) // <-- Le decimos a la barra: "Yo soy tu modelo (controlador)"
{
    // 1. Configurar y añadir la barra de menú (¡Lo primero!)
    addAndMakeVisible(menuBar);

    // --- (El resto de tu configuración de botones sigue igual...) ---
    addAndMakeVisible(toolsLabel);
    toolsLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    toolsLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(listLabel);
    listLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    listLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(canvas);

    // BOTONES
    addAndMakeVisible(addSliderBtn);
    addSliderBtn.onClick = [this] {
        static int counter = 1; 
        canvas.addElement(PluginData::ComponentType::Slider, counter, "Control " + juce::String(counter));
        counter++;
    };

    addAndMakeVisible(addToggleBtn);
    addToggleBtn.onClick = [this] {
        static int counter = 1;
        canvas.addElement(PluginData::ComponentType::Toggle, counter, "Switch " + juce::String(counter));
        counter++;
    };

    addAndMakeVisible(clearBtn);
    clearBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    clearBtn.onClick = [this] {
        propertiesPanel.inspectElement(nullptr);
        canvas.clearAll();
        project.components.clear();
    };

    addAndMakeVisible(propertiesPanel);
    
    // Conexión Canvas -> Panel
    canvas.onSelectionChanged = [this](VisualElement* el) {
        if (el) propertiesPanel.inspectElement(el);
        else    propertiesPanel.inspectProject(&project);
    };

    // Inicializar panel con proyecto
    propertiesPanel.inspectProject(&project);
    propertiesPanel.setVisible(true);

    addAndMakeVisible(generateBtn);
    generateBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    generateBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    
    generateBtn.onClick = [this] {
        canvas.updateProjectData(project);
        if (project.components.empty()) {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Vacío", "Añade algo primero.");
            return;
        }
        generator.createPluginFiles(project);
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Éxito", "Generado en Escritorio.");
    };

    setSize(950, 600);
}

MainComponent::~MainComponent() {}

// --- IMPLEMENTACIÓN DEL MENÚ ---

juce::StringArray MainComponent::getMenuBarNames()
{
    return { "Archivo", "Proyecto", "Ayuda" };
}

juce::PopupMenu MainComponent::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;

    if (menuName == "Archivo")
    {
        menu.addItem(FileNew, "Nuevo Proyecto");
        menu.addSeparator();
        menu.addItem(FileLoad, "Cargar Proyecto...", false); // Desactivado por ahora
        menu.addItem(FileSave, "Guardar Proyecto...", false); // Desactivado por ahora
        menu.addSeparator();
        menu.addItem(FileExit, "Salir");
    }
    else if (menuName == "Proyecto")
    {
        // AQUÍ ESTÁ LO QUE PEDÍAS: Cambiar datos desde el menú
        menu.addItem(ProjectProps, "Propiedades del Plugin...");
        menu.addSeparator();
        menu.addItem(ProjectGenerate, "Generar Código C++");
    }
    else if (menuName == "Ayuda")
    {
        menu.addItem(HelpAbout, "Acerca de LinuxPluginMaker...");
    }

    return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    switch (menuItemID)
    {
        case FileExit: 
            juce::JUCEApplication::getInstance()->systemRequestedQuit(); 
            break;

        case ProjectProps:
            // ESTO ES CLAVE: Forzamos al panel derecho a mostrar los datos globales
            propertiesPanel.inspectProject(&project);
            break;

        case ProjectGenerate:
            // Simulamos click en el botón naranja
            generateBtn.triggerClick(); 
            break;

        case HelpAbout:
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
                "Acerca de", "TFG: Generador de Plugins LV2 para Linux.\nVersión 0.2");
            break;
            
        default:
            break;
    }
}

// --- LAYOUT ACTUALIZADO ---

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    
    // Las líneas ahora empiezan un poco más abajo (debajo del menú)
    g.setColour(juce::Colours::black);
    g.fillRect(200, 20, 2, getHeight());       // Izquierda
    g.fillRect(getWidth() - 250, 20, 2, getHeight()); // Derecha
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // 1. MENU BAR (Ocupa la tira de arriba)
    // .removeFromTop(20) corta 20 píxeles de arriba y se los da a la barra.
    // El resto de 'area' se hace más pequeño automáticamente.
    menuBar.setBounds(area.removeFromTop(20));

    // 2. COLUMNA IZQUIERDA
    auto sidebar = area.removeFromLeft(200);
    sidebar.reduce(10, 10); 
    toolsLabel.setBounds(sidebar.removeFromTop(30));
    sidebar.removeFromTop(20); 
    addSliderBtn.setBounds(sidebar.removeFromTop(40));
    sidebar.removeFromTop(10);
    addToggleBtn.setBounds(sidebar.removeFromTop(40));
    clearBtn.setBounds(sidebar.removeFromBottom(40));

    // 3. COLUMNA DERECHA
    auto propsArea = area.removeFromRight(250); 
    propertiesPanel.setBounds(propsArea);

    // 4. CENTRO
    area.reduce(10, 10); 
    listLabel.setBounds(area.removeFromTop(30));
    generateBtn.setBounds(area.removeFromBottom(50));
    area.removeFromBottom(10);
    canvas.setBounds(area);
}