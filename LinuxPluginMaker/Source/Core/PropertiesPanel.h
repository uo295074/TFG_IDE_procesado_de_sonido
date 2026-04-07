#pragma once
#include "PluginData.h"
#include "VisualElement.h"
#include <juce_gui_basics/juce_gui_basics.h>

class PropertiesPanel : public juce::Component,
                        public juce::TextEditor::Listener,
                        public juce::ComboBox::Listener {
public:
  std::function<void()> onDataChanged;

  PropertiesPanel() {
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // --- COMPONENTE ---
    addCompField(compNameLabel, compNameEditor, "Nombre:");
    addCompField(compIDLabel, compIDEditor, "ID (Symbol):");
    addCompField(minLabel, minEditor, "Mínimo:");
    addCompField(maxLabel, maxEditor, "Máximo:");
    addCompField(defLabel, defEditor, "Defecto:");

    // --- PARAM (ANTES ROLE) ---
    roleLabel.setText("Parámetro:", juce::dontSendNotification);
    addAndMakeVisible(roleLabel);
    addAndMakeVisible(roleCombo);
    roleCombo.addListener(this);

    // --- PROYECTO ---
    addProjField(projNameLabel, projNameEditor, "Nombre Plugin:");
    addProjField(projManufLabel, projManufEditor, "Fabricante:");
    addProjField(projURILabel, projURIEditor, "URI Único:");

    inputsLabel.setText("Entradas:", juce::dontSendNotification);
    addAndMakeVisible(inputsLabel);
    addAndMakeVisible(inputsCombo);
    inputsCombo.addItem("1 (Mono)", 1);
    inputsCombo.addItem("2 (Estéreo)", 2);
    inputsCombo.addListener(this);

    outputsLabel.setText("Salidas:", juce::dontSendNotification);
    addAndMakeVisible(outputsLabel);
    addAndMakeVisible(outputsCombo);
    outputsCombo.addItem("1 (Mono)", 1);
    outputsCombo.addItem("2 (Estéreo)", 2);
    outputsCombo.addListener(this);

    algoLabel.setText("Efecto:", juce::dontSendNotification);
    addAndMakeVisible(algoLabel);

    addAndMakeVisible(algoCombo);
    algoCombo.addListener(this);
  }

  void inspectElement(VisualElement *element) {
    currentElement = element;

    updateVisibility();
    updateRoleOptions();

    if (element) {
      titleLabel.setText("EDITAR COMPONENTE", juce::dontSendNotification);

      compNameEditor.setText(element->getName());
      compIDEditor.setText(element->getSymbol());

      if (isSliderLike()) {
        minEditor.setText(juce::String(element->getMin()));
        maxEditor.setText(juce::String(element->getMax()));
        defEditor.setText(juce::String(element->getDef()));
      }

      // 🔥 NUEVO: restaurar selección del parámetro
      if (element->getParamName().isNotEmpty()) {
        roleCombo.setText(element->getParamName(), juce::dontSendNotification);
      }
    }
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
      algoCombo.setSelectedId(project->currentEffectIndex + 1,
                              juce::dontSendNotification);
    }

    updateRoleOptions();
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
    layoutField(roleLabel, roleCombo);

    layoutField(projNameLabel, projNameEditor);
    layoutField(projManufLabel, projManufEditor);
    layoutField(projURILabel, projURIEditor);

    if (algoCombo.isVisible()) {
      layoutField(inputsLabel, inputsCombo);
      layoutField(outputsLabel, outputsCombo);
      layoutField(algoLabel, algoCombo);
    }
  }

  void textEditorFocusLost(juce::TextEditor &) override { applyChanges(); }
  void textEditorReturnKeyPressed(juce::TextEditor &) override {
    applyChanges();
  }
  void comboBoxChanged(juce::ComboBox *) override { applyChanges(); }

private:
  VisualElement *currentElement = nullptr;
  PluginData::Project *currentProject = nullptr;

  juce::Label titleLabel;

  juce::Label compNameLabel, compIDLabel, minLabel, maxLabel, defLabel;
  juce::TextEditor compNameEditor, compIDEditor, minEditor, maxEditor,
      defEditor;

  juce::Label roleLabel;
  juce::ComboBox roleCombo;

  juce::Label projNameLabel, projManufLabel, projURILabel;
  juce::TextEditor projNameEditor, projManufEditor, projURIEditor;

  juce::Label inputsLabel, outputsLabel;
  juce::ComboBox inputsCombo, outputsCombo;

  juce::Label algoLabel;
  juce::ComboBox algoCombo;

  void populateAlgorithmCombo() {
    algoCombo.clear();

    if (!currentProject)
      return;

    int id = 1;
    for (auto &effect : currentProject->availableEffects) {
      algoCombo.addItem(effect.name, id++);
    }
  }

  // 🔥 CLAVE: PARAMS DESDE XML
  void updateRoleOptions() {
    roleCombo.clear();

    if (!currentProject)
      return;

    int idx = currentProject->currentEffectIndex;
    if (idx < 0 || idx >= currentProject->availableEffects.size())
      return;

    auto &effect = currentProject->availableEffects[idx];

    int id = 1;
    for (auto &param : effect.params) {
      roleCombo.addItem(param.name, id++);
    }
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

  bool isSliderLike() const {
    if (!currentElement)
      return false;

    return (currentElement->getType() == PluginData::ComponentType::Slider ||
            currentElement->getType() == PluginData::ComponentType::Knob);
  }

  void updateVisibility() {
    bool showComp = (currentElement != nullptr);
    bool showProj = (!showComp && currentProject != nullptr);
    bool slider = isSliderLike();

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
    roleLabel.setVisible(showComp && slider);
    roleCombo.setVisible(showComp && slider);

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

    resized();
  }

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

              // 🔥 NUEVO (UI moderna)
              currentElement->setParamName(paramName);

              // 🔥 CLAVE (NO ROMPER DSP)
              currentElement->setRole(getRoleFromName(paramName));
            }
          }
        }
      }

      if (onDataChanged)
        onDataChanged();
    } else if (currentProject) {
      currentProject->pluginName = projNameEditor.getText();
      currentProject->manufacturer = projManufEditor.getText();
      currentProject->pluginURI = projURIEditor.getText();

      if (inputsCombo.getSelectedId() > 0)
        currentProject->numInputs = inputsCombo.getSelectedId();

      if (outputsCombo.getSelectedId() > 0)
        currentProject->numOutputs = outputsCombo.getSelectedId();

      if (algoCombo.getSelectedId() > 0) {
        currentProject->currentEffectIndex = algoCombo.getSelectedId() - 1;
        currentProject->currentAlgorithm =
            (PluginData::AlgorithmType)currentProject->currentEffectIndex;
        updateRoleOptions();
      }
    }
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
};
