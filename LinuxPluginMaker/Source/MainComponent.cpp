/*
  ==============================================================================
    Source/MainComponent.cpp
    Componente principal del IDE y coordinacion de la interfaz.
  ==============================================================================
*/
#include "MainComponent.h"
#include "Core/CodeEditorPanel.h"
#include <cstdlib>
#include <vector>

namespace {
const juce::Colour kBgTop(0xff2e3f47);
const juce::Colour kBgBottom(0xff1f2a31);
const juce::Colour kPanelBg(0xff344750);
const juce::Colour kPanelStroke(0xff4f6772);
const juce::Colour kTextStrong(0xffecf2f6);
const juce::Colour kButtonBg(0xff2b3f48);
const juce::Colour kButtonOn(0xff365666);
const juce::Colour kGenerate(0xfff3b233);
const juce::Colour kDelete(0xffef8f1f);
const juce::Colour kDanger(0xffa82626);

bool commandExists(const juce::String &command) {
#if JUCE_LINUX || JUCE_MAC
  juce::String check = "command -v " + command + " >/dev/null 2>&1";
  return std::system(check.toRawUTF8()) == 0;
#else
  juce::ignoreUnused(command);
  return false;
#endif
}

bool lv2HeadersAvailable() {
  return juce::File("/usr/include/lv2/core/lv2.h").existsAsFile() ||
         juce::File("/usr/local/include/lv2/core/lv2.h").existsAsFile() ||
         (commandExists("pkg-config") &&
          std::system("pkg-config --exists lv2 >/dev/null 2>&1") == 0);
}

class BuildConfigPanel : public juce::Component {
public:
  BuildConfigPanel(juce::String &headers, juce::String &libraries,
                   juce::String &includePaths)
      : headersRef(headers), librariesRef(libraries), includePathsRef(includePaths) {
    title.setText(juce::String::fromUTF8("Configuración avanzada de compilación"),
                  juce::dontSendNotification);
    title.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(title);

    headersLabel.setText(juce::String::fromUTF8("Cabeceras C++ extra:"),
                         juce::dontSendNotification);
    libsLabel.setText(juce::String::fromUTF8("Librerías extra:"),
                      juce::dontSendNotification);
    includesLabel.setText("Includes extra:", juce::dontSendNotification);
    addAndMakeVisible(headersLabel);
    addAndMakeVisible(libsLabel);
    addAndMakeVisible(includesLabel);

    setupEditor(headersEditor, headersRef);
    setupEditor(libsEditor, librariesRef);
    setupEditor(includesEditor, includePathsRef);

    help.setText(juce::String::fromUTF8(
                     "Ejemplo cabeceras: #include <fftw3.h>\n"
                     "Ejemplo librerías: fftw3 sndfile\n"
                     "Ejemplo includes: /usr/include/milibreria"),
                  juce::dontSendNotification);
    help.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(help);

    saveBtn.setButtonText("Guardar y Cerrar");
    saveBtn.onClick = [this] {
      headersRef = headersEditor.getText();
      librariesRef = libsEditor.getText();
      includePathsRef = includesEditor.getText();

      if (auto *dw = findParentComponentOfClass<juce::DialogWindow>())
        dw->exitModalState(0);
    };
    addAndMakeVisible(saveBtn);
  }

  void resized() override {
    auto area = getLocalBounds().reduced(14);
    title.setBounds(area.removeFromTop(28));
    area.removeFromTop(10);

    headersLabel.setBounds(area.removeFromTop(22));
    headersEditor.setBounds(area.removeFromTop(80));
    area.removeFromTop(12);

    libsLabel.setBounds(area.removeFromTop(22));
    libsEditor.setBounds(area.removeFromTop(80));
    area.removeFromTop(12);

    includesLabel.setBounds(area.removeFromTop(22));
    includesEditor.setBounds(area.removeFromTop(80));
    area.removeFromTop(10);

    help.setBounds(area.removeFromTop(45));
    saveBtn.setBounds(area.removeFromBottom(38).removeFromRight(160));
  }

private:
  void setupEditor(juce::TextEditor &editor, const juce::String &text) {
    editor.setMultiLine(true);
    editor.setReturnKeyStartsNewLine(true);
    editor.setText(text, juce::dontSendNotification);
    addAndMakeVisible(editor);
  }

  juce::String &headersRef;
  juce::String &librariesRef;
  juce::String &includePathsRef;
  juce::Label title, headersLabel, libsLabel, includesLabel, help;
  juce::TextEditor headersEditor, libsEditor, includesEditor;
  juce::TextButton saveBtn;
};
} // namespace

// Identificadores internos de las acciones disponibles en la barra de menus.
enum MenuIDs {
  FileNew = 1,
  FileLoad,
  FileSave,
  FileExit,
  ProjectProps,
  ProjectGenerate,
  ProjectCodeEditor,
  ProjectInitEditor,
  ProjectVarsEditor,
  ProjectBuildConfig,
  ProjectExportSource,
  HelpVerifyEnvironment,
  HelpAbout
};

MainComponent::MainComponent() : menuBar(this) {
  // Carga los efectos disponibles desde el XML distribuido junto al ejecutable.
  {
    juce::File xmlFile =
        juce::File::getSpecialLocation(juce::File::currentExecutableFile)
            .getParentDirectory()
            .getChildFile("Data")
            .getChildFile("effects.xml");

    if (xmlFile.existsAsFile()) {
      project.availableEffects = PluginData::loadEffectsFromXML(xmlFile);

      // Fallback para mantener disponible Delay si se carga un XML antiguo.
      bool hasDelay = false;
      for (auto &effect : project.availableEffects)
        if (effect.name == "Delay")
          hasDelay = true;

      if (!hasDelay) {
        PluginData::EffectDefinition delay;
        delay.name = "Delay";
        delay.params.push_back({"Time"});
        delay.params.push_back({"Feedback"});
        delay.params.push_back({"Mix"});
        project.availableEffects.push_back(delay);
      }

    } else {
      juce::Logger::writeToLog("No se encontró effects.xml");
    }
  }

  // Construccion de la interfaz principal.

  addAndMakeVisible(menuBar);
  menuBar.setColour(juce::PopupMenu::backgroundColourId,
                    juce::Colour(0xff2a3a42));
  menuBar.setColour(juce::PopupMenu::textColourId, kTextStrong);
  menuBar.setColour(juce::PopupMenu::highlightedBackgroundColourId,
                    juce::Colour(0xff446273));
  menuBar.setColour(juce::PopupMenu::highlightedTextColourId,
                    juce::Colours::white);

  addAndMakeVisible(toolsLabel);
  toolsLabel.setFont(juce::Font(18.0f, juce::Font::bold));
  toolsLabel.setJustificationType(juce::Justification::centred);
  toolsLabel.setColour(juce::Label::textColourId, kTextStrong);

  addAndMakeVisible(listLabel);
  listLabel.setFont(juce::Font(18.0f, juce::Font::bold));
  listLabel.setJustificationType(juce::Justification::centred);
  listLabel.setColour(juce::Label::textColourId, kTextStrong);

  auto setupLedToggle = [this](juce::ToggleButton &toggle, bool *target) {
    addAndMakeVisible(toggle);
    toggle.setColour(juce::ToggleButton::textColourId, kTextStrong);
    toggle.onClick = [&toggle, target] { *target = toggle.getToggleState(); };
  };

  addAndMakeVisible(canvas);
  canvas.setProject(&project);

  // Herramientas del lienzo.
  addAndMakeVisible(addSliderBtn);
  addSliderBtn.setColour(juce::TextButton::buttonColourId, kButtonBg);
  addSliderBtn.setColour(juce::TextButton::buttonOnColourId, kButtonOn);
  addSliderBtn.setColour(juce::TextButton::textColourOffId, kTextStrong);
  addSliderBtn.onClick = [this] {
    static int counter = 1;
    canvas.addElement(PluginData::ComponentType::Slider, counter,
                      "Control " + juce::String(counter));
    counter++;
  };

  addAndMakeVisible(addToggleBtn);
  addToggleBtn.setColour(juce::TextButton::buttonColourId, kButtonBg);
  addToggleBtn.setColour(juce::TextButton::buttonOnColourId, kButtonOn);
  addToggleBtn.setColour(juce::TextButton::textColourOffId, kTextStrong);
  addToggleBtn.onClick = [this] {
    static int counter = 1;
    canvas.addElement(PluginData::ComponentType::Toggle, counter,
                      "Switch " + juce::String(counter));
    counter++;
  };

  addAndMakeVisible(addKnobBtn);
  addKnobBtn.setColour(juce::TextButton::buttonColourId, kButtonBg);
  addKnobBtn.setColour(juce::TextButton::buttonOnColourId, kButtonOn);
  addKnobBtn.setColour(juce::TextButton::textColourOffId, kTextStrong);
  addKnobBtn.onClick = [this] {
    static int counter = 1;
    canvas.addElement(PluginData::ComponentType::Knob, counter,
                      "Knob " + juce::String(counter));
    counter++;
  };

  addAndMakeVisible(addSelectorBtn);
  addSelectorBtn.setColour(juce::TextButton::buttonColourId, kButtonBg);
  addSelectorBtn.setColour(juce::TextButton::buttonOnColourId, kButtonOn);
  addSelectorBtn.setColour(juce::TextButton::textColourOffId, kTextStrong);
  addSelectorBtn.onClick = [this] {
    bool selectorAllowed = project.isCustom;

    if (!project.isCustom &&
        project.currentEffectIndex >= 0 &&
        project.currentEffectIndex < project.availableEffects.size()) {
      auto &effect = project.availableEffects[project.currentEffectIndex];
      selectorAllowed = !effect.selectors.empty();
    }

    if (!selectorAllowed) {
      juce::AlertWindow::showMessageBoxAsync(
          juce::AlertWindow::WarningIcon,
          juce::String::fromUTF8("Selector no disponible"),
          juce::String::fromUTF8(
              "El efecto actual no define un selector. Usa sliders/knobs o cambia a modo Custom."));
      return;
    }

    if (!project.isCustom &&
        canvas.countElementsOfType(PluginData::ComponentType::Selector) > 0) {
      juce::AlertWindow::showMessageBoxAsync(
          juce::AlertWindow::WarningIcon,
          juce::String::fromUTF8("Selector duplicado"),
          juce::String::fromUTF8(
              "Los efectos predefinidos solo utilizan un selector."));
      return;
    }

    static int counter = 1;
    canvas.addElement(PluginData::ComponentType::Selector, counter,
                      "Selector " + juce::String(counter));
    counter++;
  };

  addAndMakeVisible(clearBtn);
  clearBtn.setColour(juce::TextButton::buttonColourId, kDanger);
  clearBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
  clearBtn.onClick = [this] {
    propertiesPanel.inspectElement(nullptr);
    canvas.clearAll();
    project.components.clear();
  };

  addAndMakeVisible(deleteBtn);
  deleteBtn.setColour(juce::TextButton::buttonColourId, kDelete);
  deleteBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

  deleteBtn.onClick = [this] { canvas.deleteSelected(); };

  addAndMakeVisible(ledToolsLabel);
  ledToolsLabel.setFont(juce::Font(16.0f, juce::Font::bold));
  ledToolsLabel.setJustificationType(juce::Justification::centredLeft);
  ledToolsLabel.setColour(juce::Label::textColourId, kTextStrong);

  setupLedToggle(inputLedToggle, &project.enableInputLed);
  setupLedToggle(outputLedToggle, &project.enableOutputLed);
  setupLedToggle(clipLedToggle, &project.enableClipLed);
  setupLedToggle(levelMeterToggle, &project.enableLevelMeter);
  setupLedToggle(rmsMeterToggle, &project.enableRmsMeter);
  setupLedToggle(processingLedToggle, &project.enableProcessingLed);
  syncLedTogglesFromProject();

  addAndMakeVisible(propertiesPanel);
  propertiesPanel.inspectProject(&project);

  canvas.onSelectionChanged = [this](VisualElement *el) {
    if (el)
      propertiesPanel.inspectElement(el);
    else
      propertiesPanel.inspectProject(&project);
  };

  propertiesPanel.onDataChanged = [this]() {
    syncPresetDspCode();
    normalizeSelectorsForCurrentEffect();
    canvas.repaint(); // Actualiza la visualizacion de parametros y selectores.
    canvas.refreshAutoParams();
  };

  propertiesPanel.inspectProject(&project);
  propertiesPanel.repaint();
  propertiesPanel.setVisible(true);
  syncPresetDspCode();

  addAndMakeVisible(generateBtn);
  generateBtn.setColour(juce::TextButton::buttonColourId, kGenerate);
  generateBtn.setColour(juce::TextButton::buttonOnColourId,
                        juce::Colour(0xfff8c45c));
  generateBtn.setColour(juce::TextButton::textColourOffId,
                        juce::Colour(0xff111111));

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

// Configuracion de la barra de menus.

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
    menu.addItem(ProjectProps, "Propiedades del Plugin...");
    menu.addItem(ProjectCodeEditor,
                 juce::String::fromUTF8("Editor de Código DSP..."));
    menu.addItem(ProjectInitEditor,
                 juce::String::fromUTF8("Editor de Inicialización..."));
    menu.addItem(ProjectVarsEditor,
                 juce::String::fromUTF8("Variables DSP persistentes..."));
    menu.addItem(ProjectBuildConfig,
                 juce::String::fromUTF8("Configuración de compilación..."));
    menu.addSeparator();
    menu.addItem(ProjectGenerate, juce::String::fromUTF8("Generar Código C++"));
    menu.addItem(ProjectExportSource,
                 juce::String::fromUTF8("Exportar código fuente..."));
  } else if (menuName == "Ayuda") {
    menu.addItem(HelpVerifyEnvironment,
                 juce::String::fromUTF8("Verificación de entorno..."));
    menu.addSeparator();
    menu.addItem(HelpAbout, "Acerca de LinuxPluginMaker...");
  } else if (menuName == "Presets") {
    menu.addItem(1001, "Distortion");
    menu.addItem(1002, "Filter");
    menu.addItem(1003, "Tremolo");
    menu.addItem(1004, "Delay");
  }

  return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex) {
  switch (menuItemID) {
  case FileNew:
    promptSaveBeforeNewProject();
    break;

  case FileSave:
    saveProjectAs();
    break;

  case FileLoad:
    // Abre un proyecto XML guardado previamente.
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
                                    // Restaura el modelo del proyecto desde el XML.
                                    project.fromXml(xml.get());
                                    // Reconstruye el lienzo a partir de los componentes guardados.
                                    canvas.loadProject(project);
                                    // Sincroniza paneles y controles de monitorizacion.
                                    propertiesPanel.inspectProject(&project);
                                   syncLedTogglesFromProject();
                                 }
                               }
                             });
    break;

  case FileExit:
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
    break;

  case ProjectProps:
    // Muestra de nuevo las propiedades globales del proyecto.
    propertiesPanel.inspectProject(&project);
    break;

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

  case ProjectVarsEditor: {
    auto *editorPanel = new CodeEditorPanel(project.userVariables);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(editorPanel);
    options.content->setSize(760, 520);
    options.dialogTitle = juce::String::fromUTF8("Variables DSP persistentes");
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;

    options.launchAsync();
    break;
  }

  case ProjectBuildConfig: {
    auto *configPanel = new BuildConfigPanel(
        project.extraHeaders, project.extraLibraries, project.extraIncludePaths);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(configPanel);
    options.content->setSize(620, 500);
    options.dialogTitle = juce::String::fromUTF8("Configuración de compilación");
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;

    options.launchAsync();
    break;
  }
  case ProjectExportSource: {
    // Genera una copia actualizada antes de exportar el codigo fuente.
    canvas.updateProjectData(project);
    generator.createPluginFiles(project);

    juce::String defaultFolderName =
        PluginGenerator::getSafePluginFileName(project);

    fileChooser = std::make_unique<juce::FileChooser>(
        juce::String::fromUTF8(
            "Elige el nombre y ubicación de la carpeta exportada"),
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile(defaultFolderName),
        "*");

    fileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::canSelectDirectories,
        [this, defaultFolderName](const juce::FileChooser &fc) {
          juce::File target = fc.getResult();
          if (target != juce::File{}) {
            juce::File source = generator.getGeneratedProjectDir(project);

            if (target.exists() && target.isDirectory())
              target = target.getChildFile(defaultFolderName);

            if (target.exists()) {
              juce::File parent = target.getParentDirectory();
              juce::String baseName = target.getFileName();

              int suffix = 1;
              do {
                target =
                    parent.getChildFile(baseName + "_" + juce::String(suffix));
                suffix++;
              } while (target.exists());
            }

            if (!source.copyDirectoryTo(target)) {
              juce::AlertWindow::showMessageBoxAsync(
                  juce::AlertWindow::WarningIcon,
                  juce::String::fromUTF8("Error de exportación"),
                  juce::String::fromUTF8(
                      "No se pudo copiar el código fuente generado."));
              return;
            }

            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                juce::String::fromUTF8("Exportación completada"),
                juce::String::fromUTF8(
                    "El código fuente se ha exportado en:\n") +
                    target.getFullPathName());
          }
        });

    break;
  }

  case ProjectGenerate:
    // Reutiliza la misma ruta de generacion que el boton principal.
    generateBtn.triggerClick();
    break;

  case HelpAbout:
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon, "Acerca de",
        juce::String::fromUTF8(
            "TFG: Generador de Plugins LV2 para Linux.\nVersión 0.2"));
    break;

  case HelpVerifyEnvironment:
    verifyBuildEnvironment();
    break;

  case 1001: // Distortion
  {
    canvas.clearAll();

    for (int i = 0; i < project.availableEffects.size(); ++i)
      if (project.availableEffects[i].name == "Distortion")
        project.currentEffectIndex = i;

    project.currentAlgorithm = PluginData::AlgorithmType::Distortion;
    project.isCustom = false;
    syncPresetDspCode();

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
    syncPresetDspCode();

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
    syncPresetDspCode();

    static int id = 1;
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Rate");
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Depth");
    canvas.addElement(PluginData::ComponentType::Selector, id++, "Mode");

    propertiesPanel.inspectProject(&project);
    canvas.refreshAutoParams();
    break;
  }

  case 1004: // Delay
  {
    canvas.clearAll();

    for (int i = 0; i < project.availableEffects.size(); ++i)
      if (project.availableEffects[i].name == "Delay")
        project.currentEffectIndex = i;

    project.currentAlgorithm = PluginData::AlgorithmType::Delay;
    project.isCustom = false;
    syncPresetDspCode();

    static int id = 1;
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Time");
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Feedback");
    canvas.addElement(PluginData::ComponentType::Slider, id++, "Mix");

    propertiesPanel.inspectProject(&project);
    canvas.refreshAutoParams();
    break;
  }

  default:
    break;
  }
}

void MainComponent::paint(juce::Graphics &g) {
  g.setGradientFill(juce::ColourGradient(kBgTop, 0.0f, 0.0f, kBgBottom, 0.0f,
                                         (float)getHeight(), false));
  g.fillAll();

  auto content = getLocalBounds().withTrimmedTop(20);
  auto leftPanel = content.removeFromLeft(200).reduced(4);
  auto rightPanel = content.removeFromRight(250).reduced(4);

  g.setColour(kPanelBg.withAlpha(0.92f));
  g.fillRoundedRectangle(leftPanel.toFloat(), 10.0f);
  g.fillRoundedRectangle(rightPanel.toFloat(), 10.0f);

  g.setColour(kPanelStroke.withAlpha(0.65f));
  g.drawRoundedRectangle(leftPanel.toFloat(), 10.0f, 1.0f);
  g.drawRoundedRectangle(rightPanel.toFloat(), 10.0f, 1.0f);

  auto centerPanel = content.reduced(6);
  g.setColour(juce::Colour(0xff213039).withAlpha(0.45f));
  g.fillRoundedRectangle(centerPanel.toFloat(), 10.0f);
}

void MainComponent::resized() {
  auto area = getLocalBounds();

  // Barra superior de menus.
  menuBar.setBounds(area.removeFromTop(24));

  // Columna izquierda con herramientas e indicadores.
  auto sidebar = area.removeFromLeft(200).reduced(12, 14);
  toolsLabel.setBounds(sidebar.removeFromTop(30));
  sidebar.removeFromTop(14);
  addSliderBtn.setBounds(sidebar.removeFromTop(40));
  sidebar.removeFromTop(10);
  addToggleBtn.setBounds(sidebar.removeFromTop(40));
  sidebar.removeFromTop(10);
  addKnobBtn.setBounds(sidebar.removeFromTop(40));
  sidebar.removeFromTop(10);
  addSelectorBtn.setBounds(sidebar.removeFromTop(40));
  sidebar.removeFromTop(10);
  deleteBtn.setBounds(sidebar.removeFromTop(40));
  sidebar.removeFromTop(18);

  ledToolsLabel.setBounds(sidebar.removeFromTop(24));
  sidebar.removeFromTop(6);

  auto layoutLedToggle = [&](juce::ToggleButton &toggle) {
    toggle.setBounds(sidebar.removeFromTop(22));
    sidebar.removeFromTop(4);
  };

  layoutLedToggle(inputLedToggle);
  layoutLedToggle(outputLedToggle);
  layoutLedToggle(clipLedToggle);
  layoutLedToggle(levelMeterToggle);
  layoutLedToggle(rmsMeterToggle);
  layoutLedToggle(processingLedToggle);

  clearBtn.setBounds(sidebar.removeFromBottom(42));

  // Panel derecho de propiedades.
  auto propsArea = area.removeFromRight(250).reduced(12, 14);
  propertiesPanel.setBounds(propsArea);

  // Zona central con el lienzo y el boton de generacion.
  area.reduce(14, 14);
  listLabel.setBounds(area.removeFromTop(30));
  generateBtn.setBounds(area.removeFromBottom(46));
  area.removeFromBottom(8);
  canvas.setBounds(area);
}

void MainComponent::syncPresetDspCode() {
  if (project.isCustom)
    return;

  project.currentAlgorithm =
      (PluginData::AlgorithmType)project.currentEffectIndex;
  project.customDspCode =
      PluginGenerator::getBuiltinDspCode(project.currentAlgorithm);
}

void MainComponent::syncLedTogglesFromProject() {
  inputLedToggle.setToggleState(project.enableInputLed,
                                juce::dontSendNotification);
  outputLedToggle.setToggleState(project.enableOutputLed,
                                 juce::dontSendNotification);
  clipLedToggle.setToggleState(project.enableClipLed,
                               juce::dontSendNotification);
  levelMeterToggle.setToggleState(project.enableLevelMeter,
                                  juce::dontSendNotification);
  rmsMeterToggle.setToggleState(project.enableRmsMeter,
                                juce::dontSendNotification);
  processingLedToggle.setToggleState(project.enableProcessingLed,
                                     juce::dontSendNotification);
}

void MainComponent::normalizeSelectorsForCurrentEffect() {
  if (project.isCustom)
    return;

  bool effectUsesSelector = false;

  if (project.currentEffectIndex >= 0 &&
      project.currentEffectIndex < project.availableEffects.size()) {
    auto &effect = project.availableEffects[project.currentEffectIndex];
    effectUsesSelector = !effect.selectors.empty();
  }

  int selectorCount =
      canvas.countElementsOfType(PluginData::ComponentType::Selector);
  int maxSelectors = effectUsesSelector ? 1 : 0;

  if (selectorCount <= maxSelectors)
    return;

  canvas.limitElementsOfType(PluginData::ComponentType::Selector, maxSelectors);
  propertiesPanel.inspectProject(&project);

  juce::String message = effectUsesSelector
                             ? juce::String::fromUTF8(
                                   "El efecto actual solo utiliza un selector. Se han eliminado los selectores sobrantes.")
                             : juce::String::fromUTF8(
                                   "El efecto actual no utiliza selector. Se han eliminado los selectores del lienzo.");

  juce::AlertWindow::showMessageBoxAsync(
      juce::AlertWindow::WarningIcon,
      juce::String::fromUTF8("Selectores ajustados"), message);
}

bool MainComponent::hasUnsavedProjectChanges() {
  canvas.updateProjectData(project);

  PluginData::Project defaults;

  return !project.components.empty() || project.pluginName != defaults.pluginName ||
         project.manufacturer != defaults.manufacturer ||
         project.pluginURI != defaults.pluginURI || project.isCustom != defaults.isCustom ||
         project.currentEffectIndex != defaults.currentEffectIndex ||
         project.currentAlgorithm != defaults.currentAlgorithm ||
         project.numInputs != defaults.numInputs ||
         project.numOutputs != defaults.numOutputs ||
         project.enableBypass != defaults.enableBypass ||
         project.enableInputLed != defaults.enableInputLed ||
         project.enableOutputLed != defaults.enableOutputLed ||
         project.enableClipLed != defaults.enableClipLed ||
         project.enableLevelMeter != defaults.enableLevelMeter ||
         project.enableRmsMeter != defaults.enableRmsMeter ||
         project.enableProcessingLed != defaults.enableProcessingLed ||
         project.extraHeaders != defaults.extraHeaders ||
         project.extraLibraries != defaults.extraLibraries ||
         project.extraIncludePaths != defaults.extraIncludePaths ||
         project.userVariables != defaults.userVariables ||
         project.initCode != defaults.initCode ||
         (project.isCustom && project.customDspCode != defaults.customDspCode);
}

void MainComponent::promptSaveBeforeNewProject() {
  if (!hasUnsavedProjectChanges()) {
    resetProject();
    return;
  }

  auto options = juce::MessageBoxOptions()
                     .withIconType(juce::MessageBoxIconType::QuestionIcon)
                     .withTitle(juce::String::fromUTF8("Nuevo proyecto"))
                     .withMessage(juce::String::fromUTF8(
                         "Hay cambios en el proyecto actual. ¿Quieres guardarlo antes de crear uno nuevo?"))
                     .withButton(juce::String::fromUTF8("Guardar"))
                     .withButton(juce::String::fromUTF8("No guardar"))
                     .withButton(juce::String::fromUTF8("Cancelar"));

  juce::AlertWindow::showAsync(options, [this](int result) {
    if (result == 1) {
      saveProjectAs([this](bool saved) {
        if (saved)
          resetProject();
      });
    } else if (result == 2) {
      resetProject();
    }
  });
}

void MainComponent::resetProject() {
  auto availableEffects = project.availableEffects;

  project = PluginData::Project();
  project.availableEffects = availableEffects;

  propertiesPanel.inspectElement(nullptr);
  canvas.clearAll();
  syncPresetDspCode();
  propertiesPanel.inspectProject(&project);
  syncLedTogglesFromProject();
  canvas.repaint();
  repaint();
}

void MainComponent::saveProjectAs(std::function<void(bool)> onComplete) {
  canvas.updateProjectData(project);

  fileChooser = std::make_unique<juce::FileChooser>(
      "Guardar Proyecto",
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
      "*.xml");

  fileChooser->launchAsync(
      juce::FileBrowserComponent::saveMode |
          juce::FileBrowserComponent::canSelectFiles,
      [this, onComplete](const juce::FileChooser &fc) {
        juce::File file = fc.getResult();
        bool saved = false;

        if (file != juce::File{}) {
          if (!file.hasFileExtension("xml"))
            file = file.withFileExtension("xml");

          if (auto xml = project.toXml())
            saved = xml->writeTo(file);
        }

        if (onComplete)
          onComplete(saved);
      });
}

void MainComponent::verifyBuildEnvironment() {
  struct CheckItem {
    juce::String name;
    bool ok;
    juce::String installHint;
  };

  std::vector<CheckItem> checks;
  checks.push_back({"CMake", commandExists("cmake"),
                    "sudo apt install cmake"});
  checks.push_back({"Make", commandExists("make"),
                    "sudo apt install make"});
  checks.push_back({"Compilador g++", commandExists("g++"),
                    "sudo apt install g++"});
  checks.push_back({"Cabeceras LV2", lv2HeadersAvailable(),
                    "sudo apt install lv2-dev"});

  juce::String missing;
  juce::String okList;

  for (const auto &check : checks) {
    if (check.ok) {
      okList += "  - " + check.name + "\n";
    } else {
      missing += "  - " + check.name + "\n";
      missing += "    Instalar con: " + check.installHint + "\n";
    }
  }

  if (missing.isEmpty()) {
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        juce::String::fromUTF8("Verificación de entorno"),
        juce::String::fromUTF8(
            "El entorno está preparado para generar y compilar plug-ins LV2.\n\n") +
            juce::String::fromUTF8("Herramientas encontradas:\n") + okList);
    return;
  }

  juce::AlertWindow::showMessageBoxAsync(
      juce::AlertWindow::WarningIcon,
      juce::String::fromUTF8("Faltan herramientas"),
      juce::String::fromUTF8(
          "El sistema puede no ser capaz de compilar plug-ins LV2.\n\n") +
          juce::String::fromUTF8("Falta instalar:\n") + missing +
          juce::String::fromUTF8(
              "\nDespués de instalar las dependencias, vuelve a ejecutar esta verificación."));
}
