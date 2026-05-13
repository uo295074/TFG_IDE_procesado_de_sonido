#pragma once
#include "PluginData.h"
#include "VisualElement.h"
#include <juce_gui_basics/juce_gui_basics.h>

class PropertiesPanel : public juce::Component,
                        public juce::TextEditor::Listener,
                        public juce::ComboBox::Listener,
                        public juce::Button::Listener {
public:
  std::function<void()> onDataChanged;

  PropertiesPanel() {
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    auto styleEditor = [](juce::TextEditor &ed) {
      ed.setColour(juce::TextEditor::backgroundColourId,
                   juce::Colour(0xff243640));
      ed.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff40525c));
      ed.setColour(juce::TextEditor::focusedOutlineColourId,
                   juce::Colour(0xfff4b63b));
      ed.setColour(juce::TextEditor::textColourId, juce::Colour(0xffeef4f8));
    };

    auto styleCombo = [](juce::ComboBox &cb) {
      cb.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff243640));
      cb.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff40525c));
      cb.setColour(juce::ComboBox::textColourId, juce::Colour(0xffeef4f8));
      cb.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffd8e3ea));
    };

    auto styleToggleBtn = [](juce::TextButton &btn) {
      btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2f4754));
      btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff365868));
      btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe9f0f4));
      btn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xfff8fbff));
    };

    auto styleLabel = [](juce::Label &l) {
      l.setColour(juce::Label::textColourId, juce::Colour(0xffd3dde3));
    };

    addCompField(compNameLabel, compNameEditor, "Nombre:");
    addCompField(compIDLabel, compIDEditor, "ID (Symbol):");
    addCompField(minLabel, minEditor, juce::String::fromUTF8("Mínimo:"));
    addCompField(maxLabel, maxEditor, juce::String::fromUTF8("Máximo:"));
    addCompField(defLabel, defEditor, "Defecto:");

    addCompField(stepsLabel, stepsEditor, "Opciones:"); // Nuevo selector
    roleLabel.setText(juce::String::fromUTF8("Parámetro:"),
                      juce::dontSendNotification);
    addAndMakeVisible(roleLabel);
    addAndMakeVisible(roleCombo);
    roleCombo.addListener(this);
    styleCombo(roleCombo);

    sectionIdentityLabel.setText("Identidad", juce::dontSendNotification);
    sectionIdentityLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(sectionIdentityLabel);

    sectionRoutingLabel.setText("Audio y efecto", juce::dontSendNotification);
    sectionRoutingLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(sectionRoutingLabel);

    sectionAdvancedLabel.setText("Avanzado", juce::dontSendNotification);
    sectionAdvancedLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(sectionAdvancedLabel);

    addProjField(projNameLabel, projNameEditor, "Nombre Plugin:");
    addProjField(projManufLabel, projManufEditor, "Fabricante:");
    addProjField(projURILabel, projURIEditor,
                 juce::String::fromUTF8("URI Único:"));

    inputsLabel.setText("Entradas:", juce::dontSendNotification);
    addAndMakeVisible(inputsLabel);
    addAndMakeVisible(inputsCombo);
    inputsCombo.addItem("1 (Mono)", 1);
    inputsCombo.addItem(juce::String::fromUTF8("2 (Estéreo)"), 2);
    inputsCombo.addListener(this);
    styleCombo(inputsCombo);

    outputsLabel.setText("Salidas:", juce::dontSendNotification);
    addAndMakeVisible(outputsLabel);
    addAndMakeVisible(outputsCombo);
    outputsCombo.addItem("1 (Mono)", 1);
    outputsCombo.addItem(juce::String::fromUTF8("2 (Estéreo)"), 2);
    outputsCombo.addListener(this);
    styleCombo(outputsCombo);

    algoLabel.setText("Efecto:", juce::dontSendNotification);
    addAndMakeVisible(algoLabel);
    addAndMakeVisible(algoCombo);
    algoCombo.addListener(this);
    styleCombo(algoCombo);

    // GLOBAL BYPASS
    bypassToggle.setButtonText("Bypass global");
    bypassToggle.setColour(juce::ToggleButton::textColourId,
                           juce::Colour(0xffe5eff5));
    addAndMakeVisible(bypassToggle);
    bypassToggle.addListener(this);

    // 🔥 LED INDICATORS
    sectionIndicatorsLabel.setText("Indicadores visuales",
                                   juce::dontSendNotification);
    sectionIndicatorsLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(sectionIndicatorsLabel);

    auto setupLedToggle = [](juce::ToggleButton &toggle,
                             const juce::String &text) {
      toggle.setButtonText(text);
      toggle.setColour(juce::ToggleButton::textColourId,
                       juce::Colour(0xffe5eff5));
    };

    setupLedToggle(inputLedToggle, "Input LED");
    setupLedToggle(outputLedToggle, "Output LED");
    setupLedToggle(clipLedToggle, "Clip LED");
    setupLedToggle(levelMeterToggle, "Level Meter");
    setupLedToggle(rmsMeterToggle, "RMS Meter");
    setupLedToggle(processingLedToggle, "Processing LED");

    addAndMakeVisible(inputLedToggle);
    addAndMakeVisible(outputLedToggle);
    addAndMakeVisible(clipLedToggle);
    addAndMakeVisible(levelMeterToggle);
    addAndMakeVisible(rmsMeterToggle);
    addAndMakeVisible(processingLedToggle);

    inputLedToggle.addListener(this);
    outputLedToggle.addListener(this);
    clipLedToggle.addListener(this);
    levelMeterToggle.addListener(this);
    rmsMeterToggle.addListener(this);
    processingLedToggle.addListener(this);

    // 🔥 VARIABLES
    varsLabel.setText("Variables DSP:", juce::dontSendNotification);
    addAndMakeVisible(varsLabel);

    varsToggleBtn.setButtonText("Mostrar editor DSP");
    addAndMakeVisible(varsToggleBtn);
    varsToggleBtn.addListener(this);
    styleToggleBtn(varsToggleBtn);

    addAndMakeVisible(varsEditor);
    varsEditor.setMultiLine(true);
    varsEditor.setReturnKeyStartsNewLine(true);
    varsEditor.addListener(this);

    // 🔥 EXTRA BUILD CONFIG
    extraLibrariesLabel.setText("Librerias extra:", juce::dontSendNotification);
    addAndMakeVisible(extraLibrariesLabel);
    addAndMakeVisible(extraLibrariesEditor);
    extraLibrariesEditor.setMultiLine(true);
    extraLibrariesEditor.setReturnKeyStartsNewLine(true);
    extraLibrariesEditor.addListener(this);

    extraIncludePathsLabel.setText("Includes extra:", juce::dontSendNotification);
    addAndMakeVisible(extraIncludePathsLabel);
    addAndMakeVisible(extraIncludePathsEditor);
    extraIncludePathsEditor.setMultiLine(true);
    extraIncludePathsEditor.setReturnKeyStartsNewLine(true);
    extraIncludePathsEditor.addListener(this);

    buildToggleBtn.setButtonText("Mostrar config de compilacion");
    addAndMakeVisible(buildToggleBtn);
    buildToggleBtn.addListener(this);
    styleToggleBtn(buildToggleBtn);

    styleLabel(compNameLabel);
    styleLabel(compIDLabel);
    styleLabel(minLabel);
    styleLabel(maxLabel);
    styleLabel(defLabel);
    styleLabel(stepsLabel);
    styleLabel(roleLabel);
    styleLabel(projNameLabel);
    styleLabel(projManufLabel);
    styleLabel(projURILabel);
    styleLabel(inputsLabel);
    styleLabel(outputsLabel);
    styleLabel(algoLabel);
    styleLabel(varsLabel);
    styleLabel(extraLibrariesLabel);
    styleLabel(extraIncludePathsLabel);
    styleLabel(sectionIdentityLabel);
    styleLabel(sectionRoutingLabel);
    styleLabel(sectionIndicatorsLabel);
    styleLabel(sectionAdvancedLabel);

    styleEditor(compNameEditor);
    styleEditor(compIDEditor);
    styleEditor(minEditor);
    styleEditor(maxEditor);
    styleEditor(defEditor);
    styleEditor(stepsEditor);
    styleEditor(projNameEditor);
    styleEditor(projManufEditor);
    styleEditor(projURIEditor);
    styleEditor(varsEditor);
    styleEditor(extraLibrariesEditor);
    styleEditor(extraIncludePathsEditor);
  }

  void inspectProject(PluginData::Project *project) {
    currentProject = project;
    currentElement = nullptr;

    updateVisibility();

    if (project) {
      titleLabel.setText("AJUSTES DE PROYECTO", juce::dontSendNotification);

      projNameEditor.setText(project->pluginName);
      projManufEditor.setText(project->manufacturer);
      projURIEditor.setText(project->pluginURI);

      inputsCombo.setSelectedId(project->numInputs, juce::dontSendNotification);
      outputsCombo.setSelectedId(project->numOutputs,
                                 juce::dontSendNotification);

      populateAlgorithmCombo();

      if (project->isCustom &&
          algoCombo.getNumItems() > project->availableEffects.size())
        algoCombo.setSelectedId(999, juce::dontSendNotification);
      else if (project->availableEffects.size() > 0)
        algoCombo.setSelectedId(project->currentEffectIndex + 1,
                                juce::dontSendNotification);

      bypassToggle.setToggleState(project->enableBypass,
                                  juce::dontSendNotification);

      varsEditor.setText(project->userVariables);
      extraLibrariesEditor.setText(project->extraLibraries);
      extraIncludePathsEditor.setText(project->extraIncludePaths);
      inputLedToggle.setToggleState(project->enableInputLed,
                                    juce::dontSendNotification);
      outputLedToggle.setToggleState(project->enableOutputLed,
                                     juce::dontSendNotification);
      clipLedToggle.setToggleState(project->enableClipLed,
                                   juce::dontSendNotification);
      levelMeterToggle.setToggleState(project->enableLevelMeter,
                                      juce::dontSendNotification);
      rmsMeterToggle.setToggleState(project->enableRmsMeter,
                                    juce::dontSendNotification);
      processingLedToggle.setToggleState(project->enableProcessingLed,
                                         juce::dontSendNotification);
    }

    updateRoleOptions();
  }

  void inspectElement(VisualElement *element) {
    currentElement = element;
    updateVisibility();

    if (element) {
      titleLabel.setText("EDITAR COMPONENTE", juce::dontSendNotification);

      compNameEditor.setText(element->getName());
      compIDEditor.setText(element->getSymbol());

      if (isSliderLike()) {
        minEditor.setText(juce::String(element->getMin()));
        maxEditor.setText(juce::String(element->getMax()));
        defEditor.setText(juce::String(element->getDef()));
      }

      // 🔥 SELECTOR
      if (isSelector())
        stepsEditor.setText(juce::String(element->getNumSteps()));

      if (element->getParamName().isNotEmpty())
        roleCombo.setText(element->getParamName(), juce::dontSendNotification);
    }
  }

private:
  VisualElement *currentElement = nullptr;
  PluginData::Project *currentProject = nullptr;

  juce::Label titleLabel;

  juce::Label compNameLabel, compIDLabel, minLabel, maxLabel, defLabel;
  juce::TextEditor compNameEditor, compIDEditor, minEditor, maxEditor,
      defEditor;

  // Nuevo selector
  juce::Label stepsLabel;
  juce::TextEditor stepsEditor;

  juce::Label roleLabel;
  juce::ComboBox roleCombo;

  juce::Label projNameLabel, projManufLabel, projURILabel;
  juce::TextEditor projNameEditor, projManufEditor, projURIEditor;

  juce::Label sectionIdentityLabel, sectionRoutingLabel, sectionAdvancedLabel;

  juce::Label inputsLabel, outputsLabel;
  juce::ComboBox inputsCombo, outputsCombo;

  juce::Label algoLabel;
  juce::ComboBox algoCombo;

  // GLOBAL BYPASS
  juce::ToggleButton bypassToggle;

  // 🔥 LED INDICATORS
  juce::Label sectionIndicatorsLabel;
  juce::ToggleButton inputLedToggle, outputLedToggle, clipLedToggle,
      levelMeterToggle, rmsMeterToggle, processingLedToggle;

  juce::Label varsLabel;
  juce::TextButton varsToggleBtn;
  juce::TextEditor varsEditor;

  // 🔥 EXTRA BUILD CONFIG
  juce::TextButton buildToggleBtn;
  juce::Label extraLibrariesLabel, extraIncludePathsLabel;
  juce::TextEditor extraLibrariesEditor, extraIncludePathsEditor;

  bool showVarsEditor = false;
  bool showBuildEditors = false;

  // 🔥 FIX CRÍTICO (FALTABA)
  bool isSliderLike() const {
    if (!currentElement)
      return false;

    return (currentElement->getType() == PluginData::ComponentType::Slider ||
            currentElement->getType() == PluginData::ComponentType::Knob);
  }

  // Nuevo selector
  bool isSelector() const {
    if (!currentElement)
      return false;

    return currentElement->getType() == PluginData::ComponentType::Selector;
  }

  void populateAlgorithmCombo() {
    algoCombo.clear();
    if (!currentProject)
      return;

    int id = 1;

    for (auto &effect : currentProject->availableEffects)
      algoCombo.addItem(effect.name, id++);

    // 🔥 SOLO añadir Custom si hay efectos cargados
    if (!currentProject->availableEffects.empty())
      algoCombo.addItem("Custom", 999);
  }

  void updateRoleOptions() {
    roleCombo.clear();

    if (!currentProject)
      return;

    // 🔥 SOLO bloquear si hay efectos Y estamos en custom
    if (currentProject->isCustom && !currentProject->availableEffects.empty())
      return;

    int idx = currentProject->currentEffectIndex;

    if (idx < 0 || idx >= currentProject->availableEffects.size())
      return;

    auto &effect = currentProject->availableEffects[idx];

    int id = 1;
    for (auto &param : effect.params)
      roleCombo.addItem(param.name, id++);
  }

  void updateVisibility() {
    bool showComp = (currentElement != nullptr);
    bool showProj = (!showComp && currentProject != nullptr);
    bool slider = isSliderLike();

    // ===== COMPONENTE =====
    compNameLabel.setVisible(showComp);
    compNameEditor.setVisible(showComp);
    compIDLabel.setVisible(showComp);
    compIDEditor.setVisible(showComp);

    minLabel.setVisible(showComp && slider);
    minEditor.setVisible(showComp && slider);
    maxLabel.setVisible(showComp && slider);
    maxEditor.setVisible(showComp && slider);
    defLabel.setVisible(showComp && slider);
    defEditor.setVisible(showComp && slider);

    // 🔥 SELECTOR
    stepsLabel.setVisible(showComp && isSelector());
    stepsEditor.setVisible(showComp && isSelector());

    roleLabel.setVisible(showComp && slider);
    roleCombo.setVisible(showComp && slider);

    // ===== PROYECTO =====
    projNameLabel.setVisible(showProj);
    projNameEditor.setVisible(showProj);
    projManufLabel.setVisible(showProj);
    projManufEditor.setVisible(showProj);
    projURILabel.setVisible(showProj);
    projURIEditor.setVisible(showProj);
    sectionIdentityLabel.setVisible(showProj);

    inputsLabel.setVisible(showProj);
    inputsCombo.setVisible(showProj);
    outputsLabel.setVisible(showProj);
    outputsCombo.setVisible(showProj);

    algoLabel.setVisible(showProj);
    algoCombo.setVisible(showProj);
    sectionRoutingLabel.setVisible(showProj);
    bypassToggle.setVisible(showProj);

    // 🔥 LED INDICATORS (se configuran desde el panel izquierdo)
    sectionIndicatorsLabel.setVisible(false);
    inputLedToggle.setVisible(false);
    outputLedToggle.setVisible(false);
    clipLedToggle.setVisible(false);
    levelMeterToggle.setVisible(false);
    rmsMeterToggle.setVisible(false);
    processingLedToggle.setVisible(false);

    varsLabel.setVisible(showProj);
    varsToggleBtn.setVisible(showProj);
    varsEditor.setVisible(showProj && showVarsEditor);

    // 🔥 EXTRA BUILD CONFIG
    buildToggleBtn.setVisible(showProj);
    extraLibrariesLabel.setVisible(showProj && showBuildEditors);
    extraLibrariesEditor.setVisible(showProj && showBuildEditors);
    extraIncludePathsLabel.setVisible(showProj && showBuildEditors);
    extraIncludePathsEditor.setVisible(showProj && showBuildEditors);
    sectionAdvancedLabel.setVisible(showProj);

    resized();
  }

  // 🔥 SOLO UNA VERSIÓN (quitado duplicado)
  void applyChanges() {
    if (currentElement) {
      currentElement->setName(compNameEditor.getText());
      currentElement->setSymbol(compIDEditor.getText());

      if (isSliderLike()) {
        currentElement->setRange(minEditor.getText().getFloatValue(),
                                 maxEditor.getText().getFloatValue(),
                                 defEditor.getText().getFloatValue());

        int roleId = roleCombo.getSelectedId();

        if (roleId > 0 && currentProject) {
          int idx = currentProject->currentEffectIndex;

          if (idx >= 0 && idx < currentProject->availableEffects.size()) {
            auto &effect = currentProject->availableEffects[idx];

            if (roleId - 1 < effect.params.size()) {
              juce::String paramName = effect.params[roleId - 1].name;

              currentElement->setParamName(paramName);
              currentElement->setRole(getRoleFromName(paramName));
            }
          }
        }
      }

      // 🔥 SELECTOR
      if (isSelector()) {
        int steps = stepsEditor.getText().getIntValue();
        if (steps < 2)
          steps = 2;
        currentElement->setNumSteps(steps);
      }

    } else if (currentProject) {
      currentProject->pluginName = projNameEditor.getText();
      currentProject->manufacturer = projManufEditor.getText();
      currentProject->pluginURI = projURIEditor.getText();
      currentProject->userVariables = varsEditor.getText();
      currentProject->extraLibraries = extraLibrariesEditor.getText();
      currentProject->extraIncludePaths = extraIncludePathsEditor.getText();
      currentProject->enableBypass = bypassToggle.getToggleState();
      if (inputsCombo.getSelectedId() > 0)
        currentProject->numInputs = inputsCombo.getSelectedId();

      if (outputsCombo.getSelectedId() > 0)
        currentProject->numOutputs = outputsCombo.getSelectedId();

      int selected = algoCombo.getSelectedId();

      if (selected == 999) {
        currentProject->isCustom = true;
      } else if (selected > 0) {
        currentProject->isCustom = false;
        currentProject->currentEffectIndex = selected - 1;
      }
    }

    currentProject->currentAlgorithm =
        (PluginData::AlgorithmType)currentProject->currentEffectIndex;
    // 🔥 ESTO FALTABA
    updateRoleOptions();

    if (onDataChanged)
      onDataChanged();
  }

  PluginData::ParamRole getRoleFromName(const juce::String &name) {
    using namespace PluginData;

    if (name == "Gain")
      return ParamRole::Gain;
    if (name == "Drive")
      return ParamRole::Drive;
    if (name == "Mix")
      return ParamRole::Mix;
    if (name == "Tone")
      return ParamRole::Tone;
    if (name == "Frequency")
      return ParamRole::Frequency;
    if (name == "Depth")
      return ParamRole::Depth;

    return ParamRole::None;
  }

  void comboBoxChanged(juce::ComboBox *) override { applyChanges(); }
  void textEditorFocusLost(juce::TextEditor &) override { applyChanges(); }
  void textEditorReturnKeyPressed(juce::TextEditor &) override {
    applyChanges();
  }
  void buttonClicked(juce::Button *button) override {
    if (button == &varsToggleBtn) {
      showVarsEditor = !showVarsEditor;
      varsToggleBtn.setButtonText(showVarsEditor ? "Ocultar editor DSP"
                                                 : "Mostrar editor DSP");
      updateVisibility();
      repaint();
      return;
    }

    if (button == &buildToggleBtn) {
      showBuildEditors = !showBuildEditors;
      buildToggleBtn.setButtonText(showBuildEditors
                                       ? "Ocultar config de compilacion"
                                       : "Mostrar config de compilacion");
      updateVisibility();
      repaint();
      return;
    }

    applyChanges();
  }

  void addCompField(juce::Label &l, juce::TextEditor &e,
                    const juce::String &t) {
    l.setText(t, juce::dontSendNotification);
    addAndMakeVisible(l);
    addAndMakeVisible(e);
    e.addListener(this);
  }

  void addProjField(juce::Label &l, juce::TextEditor &e,
                    const juce::String &t) {
    l.setText(t, juce::dontSendNotification);
    addAndMakeVisible(l);
    addAndMakeVisible(e);
    e.addListener(this);
  }

  void resized() override {
    auto area = getLocalBounds().reduced(10);

    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(20);

    int h = 25, gap = 10;

    auto layoutField = [&](juce::Component &l, juce::Component &e) {
      if (e.isVisible()) {
        auto row = area.removeFromTop(h);
        l.setBounds(row.removeFromLeft(90));
        e.setBounds(row);
        area.removeFromTop(gap);
      }
    };

    layoutField(compNameLabel, compNameEditor);
    layoutField(compIDLabel, compIDEditor);
    layoutField(minLabel, minEditor);
    layoutField(maxLabel, maxEditor);
    layoutField(defLabel, defEditor);
    // 🔥 NUEVO
    layoutField(stepsLabel, stepsEditor);
    layoutField(roleLabel, roleCombo);

    if (sectionIdentityLabel.isVisible()) {
      sectionIdentityLabel.setBounds(area.removeFromTop(22));
      area.removeFromTop(4);
    }

    layoutField(projNameLabel, projNameEditor);
    layoutField(projManufLabel, projManufEditor);
    layoutField(projURILabel, projURIEditor);

    area.removeFromTop(6);

    if (sectionRoutingLabel.isVisible()) {
      sectionRoutingLabel.setBounds(area.removeFromTop(22));
      area.removeFromTop(4);
    }

    if (algoCombo.isVisible()) {
      layoutField(inputsLabel, inputsCombo);
      layoutField(outputsLabel, outputsCombo);
      layoutField(algoLabel, algoCombo);

      if (bypassToggle.isVisible()) {
        auto row = area.removeFromTop(h);
        bypassToggle.setBounds(row);
        area.removeFromTop(gap);
      }

      if (sectionIndicatorsLabel.isVisible()) {
        area.removeFromTop(4);
        sectionIndicatorsLabel.setBounds(area.removeFromTop(22));
        area.removeFromTop(4);

        auto ledRow1 = area.removeFromTop(h);
        inputLedToggle.setBounds(ledRow1.removeFromLeft(110));
        outputLedToggle.setBounds(ledRow1);
        area.removeFromTop(4);

        auto ledRow2 = area.removeFromTop(h);
        clipLedToggle.setBounds(ledRow2.removeFromLeft(110));
        levelMeterToggle.setBounds(ledRow2);
        area.removeFromTop(4);

        auto ledRow3 = area.removeFromTop(h);
        rmsMeterToggle.setBounds(ledRow3.removeFromLeft(110));
        processingLedToggle.setBounds(ledRow3);
        area.removeFromTop(gap);
      }
    }

    area.removeFromTop(6);

    if (sectionAdvancedLabel.isVisible()) {
      sectionAdvancedLabel.setBounds(area.removeFromTop(22));
      area.removeFromTop(4);
    }

    // 🔥 VARIABLES (si las tienes)
    if (varsLabel.isVisible()) {
      auto row = area.removeFromTop(25);
      varsLabel.setBounds(row.removeFromLeft(110));
      varsToggleBtn.setBounds(row);
      area.removeFromTop(gap);
    }

    if (varsEditor.isVisible()) {
      auto row = area.removeFromTop(120);
      varsEditor.setBounds(row);
      area.removeFromTop(gap);
    }

    // 🔥 EXTRA BUILD CONFIG
    if (buildToggleBtn.isVisible()) {
      auto row = area.removeFromTop(25);
      buildToggleBtn.setBounds(row);
      area.removeFromTop(gap);
    }

    if (extraLibrariesEditor.isVisible()) {
      auto row = area.removeFromTop(90);
      extraLibrariesLabel.setBounds(row.removeFromTop(20));
      extraLibrariesEditor.setBounds(row);
      area.removeFromTop(gap);
    }

    if (extraIncludePathsEditor.isVisible()) {
      auto row = area.removeFromTop(90);
      extraIncludePathsLabel.setBounds(row.removeFromTop(20));
      extraIncludePathsEditor.setBounds(row);
    }
  }
};
