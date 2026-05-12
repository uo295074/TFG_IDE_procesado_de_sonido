/*
  ==============================================================================
    Source/MainComponent.cpp
    Actualizado con Editor de Código DSP Modal + XML Effects Loader
  ==============================================================================
*/
#include "MainComponent.h"
#include "Core/CodeEditorPanel.h"

// Definimos IDs para los menús
enum MenuIDs {
  FileNew = 1,
  FileLoad,
  FileSave,
  FileExit,
  ProjectProps,
  ProjectGenerate,
  ProjectCodeEditor,
  ProjectInitEditor,
  ProjectExportSource,
  HelpAbout
};

MainComponent::MainComponent() : menuBar(this) {
  // ================================
  // 🔥 NUEVO: CARGA DE EFFECTS.XML
  // ================================
  {
    juce::File xmlFile =
        juce::File::getSpecialLocation(juce::File::currentExecutableFile)
            .getParentDirectory()
            .getChildFile("Data")
            .getChildFile("effects.xml");
    juce::Logger::writeToLog("Ruta XML: " + xmlFile.getFullPathName());

    if (xmlFile.existsAsFile()) {
      project.availableEffects = PluginData::loadEffectsFromXML(xmlFile);

      // 🔥🔥🔥 NUEVO: log también selectores
      for (auto &e : project.availableEffects) {
        juce::Logger::writeToLog("Effect: " + e.name);

        for (auto &p : e.params)
          juce::Logger::writeToLog("  Param: " + p.name);

        for (auto &s : e.selectors) {
          juce::Logger::writeToLog("  Selector: " + s.name);
          for (auto &opt : s.options)
            juce::Logger::writeToLog("    Option: " + opt);
        }
      }

    } else {
      juce::Logger::writeToLog("No se encontró effects.xml");
    }
  }

  // ================================
  // UI NORMAL (igual que antes)
  // ================================

  addAndMakeVisible(menuBar);

  addAndMakeVisible(toolsLabel);
  toolsLabel.setFont(juce::Font(18.0f, juce::Font::bold));
  toolsLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(listLabel);
  listLabel.setFont(juce::Font(18.0f, juce::Font::bold));
  listLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(canvas);
  canvas.setProject(&project);

  // BOTONES
  addAndMakeVisible(addSliderBtn);
  addSliderBtn.onClick = [this] {
    static int counter = 1;
    canvas.addElement(PluginData::ComponentType::Slider, counter,
                      "Control " + juce::String(counter));
    counter++;
  };

  addAndMakeVisible(addToggleBtn);
  addToggleBtn.onClick = [this] {
    static int counter = 1;
    canvas.addElement(PluginData::ComponentType::Toggle, counter,
                      "Switch " + juce::String(counter));
    counter++;
  };

  addAndMakeVisible(addKnobBtn);
  addKnobBtn.onClick = [this] {
    static int counter = 1;
    canvas.addElement(PluginData::ComponentType::Knob, counter,
                      "Knob " + juce::String(counter));
    counter++;
  };

  addAndMakeVisible(addSelectorBtn);
  addSelectorBtn.onClick = [this] {
    static int counter = 1;
    canvas.addElement(PluginData::ComponentType::Selector, counter,
                      "Selector " + juce::String(counter));
    counter++;
  };

  addAndMakeVisible(clearBtn);
  clearBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
  clearBtn.onClick = [this] {
    propertiesPanel.inspectElement(nullptr);
    canvas.clearAll();
    project.components.clear();
  };

  addAndMakeVisible(deleteBtn);
  deleteBtn.setColour(juce::TextButton::buttonColourId,
                      juce::Colours::darkorange);

  deleteBtn.onClick = [this] { canvas.deleteSelected(); };

  addAndMakeVisible(propertiesPanel);
  propertiesPanel.inspectProject(&project);

  canvas.onSelectionChanged = [this](VisualElement *el) {
    if (el)
      propertiesPanel.inspectElement(el);
    else
      propertiesPanel.inspectProject(&project);
  };

  propertiesPanel.onDataChanged = [this]() {
    canvas.repaint(); // 🔥🔥🔥 refresh selector dinámico
    canvas.refreshAutoParams();
  };

  propertiesPanel.inspectProject(&project);
  propertiesPanel.repaint();
  propertiesPanel.setVisible(true);

  addAndMakeVisible(generateBtn);
  generateBtn.setColour(juce::TextButton::buttonColourId,
                        juce::Colours::orange);
  generateBtn.setColour(juce::TextButton::textColourOffId,
                        juce::Colours::black);

  generateBtn.onClick = [this] {
    canvas.updateProjectData(project);

    if (project.components.empty()) {
      juce::AlertWindow::showMessageBoxAsync(
          juce::AlertWindow::WarningIcon,
          juce::String::fromUTF8("Proyecto Vacío"),
          juce::String::fromUTF8("Añade algún componente antes de generar."));
      return;
    }

    juce::String error;

    if (!generator.validateProject(project, error)) {
      juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                             "Error en el proyecto", error);
      return;
    }
    generator.createPluginFiles(project);

    generateBtn.setButtonText(
        juce::String::fromUTF8("Compilando de fondo... ¡Espera!"));
    generateBtn.setEnabled(false);

    std::thread([this]() {
      juce::String result = generator.compileAndInstallPlugin(project);

      juce::MessageManager::callAsync([this, result]() {
        generateBtn.setButtonText("GENERAR PROYECTO LV2");
        generateBtn.setEnabled(true);

        if (result.startsWith("OK:")) {
          juce::String folderName = result.substring(3);
          juce::AlertWindow::showMessageBoxAsync(
              juce::AlertWindow::InfoIcon,
              juce::String::fromUTF8("¡Éxito Total!"),
              "Plugin generado correctamente.\n\n"
              "~/.lv2/" +
                  folderName + ".lv2");
        } else {
          juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                 "Error al Compilar", result);
        }
      });
    }).detach();
  };

  setSize(950, 600);
}

MainComponent::~MainComponent() {}

// --- IMPLEMENTACIÓN DEL MENÚ ---

juce::StringArray MainComponent::getMenuBarNames() {
  return {"Archivo", "Proyecto", "Presets", "Ayuda"};
}

juce::PopupMenu MainComponent::getMenuForIndex(int topLevelMenuIndex,
                                               const juce::String &menuName) {
  juce::PopupMenu menu;

  if (menuName == "Archivo") {
    menu.addItem(FileNew, "Nuevo Proyecto");
    menu.addSeparator();
    menu.addItem(FileLoad, "Cargar Proyecto...");
    menu.addItem(FileSave, "Guardar Proyecto...");
    menu.addSeparator();
    menu.addItem(FileExit, "Salir");
  } else if (menuName == "Proyecto") {
    // AQUÍ ESTÁ LO QUE PEDÍAS: Cambiar datos desde el menú
    menu.addItem(ProjectProps, "Propiedades del Plugin...");
    menu.addItem(ProjectCodeEditor,
                 juce::String::fromUTF8("Editor de Código DSP..."));
    menu.addItem(ProjectInitEditor,
                 juce::String::fromUTF8("Editor de Inicialización..."));
    menu.addSeparator();
    menu.addItem(ProjectGenerate, juce::String::fromUTF8("Generar Código C++"));
    menu.addItem(ProjectExportSource,
                 juce::String::fromUTF8("Exportar código fuente..."));
  } else if (menuName == "Ayuda") {
    menu.addItem(HelpAbout, "Acerca de LinuxPluginMaker...");
  } else if (menuName == "Presets") {
    menu.addItem(1001, "Distortion");
    menu.addItem(1002, "Filter");
    menu.addItem(1003, "Tremolo");
  }

  return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex) {
  switch (menuItemID) {
  case FileSave:
    // 1. Asegurarnos de tener los datos
    canvas.updateProjectData(project);

    // 2. Abrir ventana
    fileChooser = std::make_unique<juce::FileChooser>(
        "Guardar Proyecto",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.xml");

    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode |
                                 juce::FileBrowserComponent::canSelectFiles,
                             [this](const juce::FileChooser &fc) {
                               juce::File file = fc.getResult();
                               if (file != juce::File{}) {

                                 // --- ESTE ES EL ARREGLO ---
                                 // Si el usuario no escribió ".xml", se lo
                                 // ponemos nosotros a la fuerza
                                 if (!file.hasFileExtension("xml"))
                                   file = file.withFileExtension("xml");
                                 // --------------------------

                                 if (auto xml = project.toXml())
                                   xml->writeTo(file);
                               }
                             });
    break;

  case FileLoad:
    // 1. Abrir ventana de carga
    fileChooser = std::make_unique<juce::FileChooser>(
        "Cargar Proyecto",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.xml");

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                 juce::FileBrowserComponent::canSelectFiles,
                             [this](const juce::FileChooser &fc) {
                               juce::File file = fc.getResult();
                               if (file != juce::File{}) {
                                 juce::XmlDocument xmlDoc(file);
                                 if (auto xml = xmlDoc.getDocumentElement()) {
                                   // 2. Extraer los datos del XML al Cerebro
                                   project.fromXml(xml.get());
                                   // 3. Redibujar la interfaz
                                   canvas.loadProject(project);
                                   // 4. Actualizar el panel derecho
                                   propertiesPanel.inspectProject(&project);
                                 }
                               }
                             });
    break;

  case FileExit:
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
    break;

  case ProjectProps:
    // ESTO ES CLAVE: Forzamos al panel derecho a mostrar los datos globales
    propertiesPanel.inspectProject(&project);
    break;

  // --- NUEVO CASO: ABRIR EL EDITOR MODAL ---
  case ProjectCodeEditor: {
    auto *editorPanel = new CodeEditorPanel(project.customDspCode);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(editorPanel);
    options.content->setSize(700, 500);
    options.dialogTitle =
        juce::String::fromUTF8("Editor de Código C++ Personalizado");
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;

    options.launchAsync();
    break;
  }

  case ProjectInitEditor: {
    auto *editorPanel = new CodeEditorPanel(project.initCode);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(editorPanel);
    options.content->setSize(700, 500);
    options.dialogTitle =
        juce::String::fromUTF8("Editor de Código de Inicialización");
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;

    options.launchAsync();
    break;
  }
  // -----------------------------------------
  case ProjectExportSource: {
    // Primero generamos el proyecto por si aún no existe
    canvas.updateProjectData(project);
    generator.createPluginFiles(project);

    fileChooser = std::make_unique<juce::FileChooser>(
        "Selecciona carpeta destino",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*");

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectDirectories,
        [this](const juce::FileChooser &fc) {
          juce::File target = fc.getResult();
          if (target != juce::File{}) {
            juce::File source =
                juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                    .getChildFile("MiEfectoDSP");

            source.copyDirectoryTo(target.getChildFile("MiEfectoDSP"));

            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                juce::String::fromUTF8("Exportación completada"),
                juce::String::fromUTF8(
                    "El código fuente se ha exportado correctamente."));
          }
        });

    break;
  }

  case ProjectGenerate:
    // Simulamos click en el botón naranja
    generateBtn.triggerClick();
    break;

  case HelpAbout:
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon, "Acerca de",
        juce::String::fromUTF8(
            "TFG: Generador de Plugins LV2 para Linux.\nVersión 0.2"));
    break;

  case 1001: // Distortion
  {
    canvas.clearAll();

    for (int i = 0; i < project.availableEffects.size(); ++i)
      if (project.availableEffects[i].name == "Distortion")
        project.currentEffectIndex = i;

    project.currentAlgorithm = PluginData::AlgorithmType::Distortion;
    project.isCustom = false;

    static int id = 1;
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Drive");
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Mix");
    canvas.addElement(PluginData::ComponentType::Selector, id++, "Mode");

    propertiesPanel.inspectProject(&project);
    canvas.refreshAutoParams();
    break;
  }

  case 1002: // Filter
  {
    canvas.clearAll();

    for (int i = 0; i < project.availableEffects.size(); ++i)
      if (project.availableEffects[i].name == "Filter")
        project.currentEffectIndex = i;

    project.currentAlgorithm = PluginData::AlgorithmType::Filter;
    project.isCustom = false;

    static int id = 1;
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Cutoff");
    canvas.addElement(PluginData::ComponentType::Selector, id++, "Mode");

    propertiesPanel.inspectProject(&project);
    canvas.refreshAutoParams();
    break;
  }

  case 1003: // Tremolo
  {
    canvas.clearAll();

    for (int i = 0; i < project.availableEffects.size(); ++i)
      if (project.availableEffects[i].name == "Tremolo")
        project.currentEffectIndex = i;

    project.currentAlgorithm = PluginData::AlgorithmType::Tremolo;
    project.isCustom = false;

    static int id = 1;
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Rate");
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Depth");
    canvas.addElement(PluginData::ComponentType::Selector, id++, "Mode");

    propertiesPanel.inspectProject(&project);
    canvas.refreshAutoParams();
    break;
  }

  default:
    break;
  }
}

// --- LAYOUT ACTUALIZADO ---

void MainComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::darkgrey);

  // Las líneas ahora empiezan un poco más abajo (debajo del menú)
  g.setColour(juce::Colours::black);
  g.fillRect(200, 20, 2, getHeight());              // Izquierda
  g.fillRect(getWidth() - 250, 20, 2, getHeight()); // Derecha
}

void MainComponent::resized() {
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
  sidebar.removeFromTop(10);
  addKnobBtn.setBounds(sidebar.removeFromTop(40));
  sidebar.removeFromTop(10);
  addSelectorBtn.setBounds(sidebar.removeFromTop(40));
  sidebar.removeFromTop(10);
  deleteBtn.setBounds(sidebar.removeFromTop(40)); // 🔥 NUEVO
  sidebar.removeFromTop(10);
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