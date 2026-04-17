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

    outputsLabel.setText("Salidas:", juce::dontSendNotification);
    addAndMakeVisible(outputsLabel);
    addAndMakeVisible(outputsCombo);
    outputsCombo.addItem("1 (Mono)", 1);
    outputsCombo.addItem(juce::String::fromUTF8("2 (Estéreo)"), 2);
    outputsCombo.addListener(this);

    algoLabel.setText("Efecto:", juce::dontSendNotification);
    addAndMakeVisible(algoLabel);
    addAndMakeVisible(algoCombo);
    algoCombo.addListener(this);

    // 🔥 VARIABLES
    varsLabel.setText("Variables DSP:", juce::dontSendNotification);
    addAndMakeVisible(varsLabel);

    addAndMakeVisible(varsEditor);
    varsEditor.setMultiLine(true);
    varsEditor.setReturnKeyStartsNewLine(true);
    varsEditor.addListener(this);
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

      varsEditor.setText(project->userVariables);
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

  juce::Label inputsLabel, outputsLabel;
  juce::ComboBox inputsCombo, outputsCombo;

  juce::Label algoLabel;
  juce::ComboBox algoCombo;

  juce::Label varsLabel;
  juce::TextEditor varsEditor;

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

    inputsLabel.setVisible(showProj);
    inputsCombo.setVisible(showProj);
    outputsLabel.setVisible(showProj);
    outputsCombo.setVisible(showProj);

    algoLabel.setVisible(showProj);
    algoCombo.setVisible(showProj);

    varsLabel.setVisible(showProj);
    varsEditor.setVisible(showProj);

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
  void buttonClicked(juce::Button *) override { applyChanges(); }

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

    layoutField(projNameLabel, projNameEditor);
    layoutField(projManufLabel, projManufEditor);
    layoutField(projURILabel, projURIEditor);

    if (algoCombo.isVisible()) {
      layoutField(inputsLabel, inputsCombo);
      layoutField(outputsLabel, outputsCombo);
      layoutField(algoLabel, algoCombo);
    }

    // 🔥 VARIABLES (si las tienes)
    if (varsEditor.isVisible()) {
      auto row = area.removeFromTop(120);
      varsLabel.setBounds(row.removeFromTop(20));
      varsEditor.setBounds(row);
    }
  }
};