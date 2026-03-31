#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "VisualElement.h"
#include "PluginData.h"

class PropertiesPanel : public juce::Component,
                        public juce::TextEditor::Listener,
                        public juce::ComboBox::Listener
{
public:
    std::function<void()> onDataChanged;

    PropertiesPanel()
    {
        titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(titleLabel);

        // --- COMPONENTE ---
        addCompField(compNameLabel, compNameEditor, "Nombre:");
        addCompField(compIDLabel, compIDEditor, "ID (Symbol):");
        addCompField(minLabel, minEditor, "Mínimo:");
        addCompField(maxLabel, maxEditor, "Máximo:");
        addCompField(defLabel, defEditor, "Defecto:");

        // --- ROLE ---
        roleLabel.setText("Tipo:", juce::dontSendNotification);
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

        algoLabel.setText("Algoritmo DSP:", juce::dontSendNotification);
        addAndMakeVisible(algoLabel);

        addAndMakeVisible(algoCombo);
        algoCombo.addItem(PluginData::getAlgorithmName(PluginData::AlgorithmType::Gain), 1);
        algoCombo.addItem(PluginData::getAlgorithmName(PluginData::AlgorithmType::Distortion), 2);
        algoCombo.addItem(PluginData::getAlgorithmName(PluginData::AlgorithmType::Tremolo), 3);
        algoCombo.addListener(this);
    }

    void inspectElement(VisualElement* element)
    {
        currentElement = element;

        updateVisibility();
        updateRoleOptions();

        if (element)
        {
            titleLabel.setText("EDITAR COMPONENTE", juce::dontSendNotification);

            compNameEditor.setText(element->getName());
            compIDEditor.setText(element->getSymbol());

            if (isSliderLike())
            {
                minEditor.setText(juce::String(element->getMin()));
                maxEditor.setText(juce::String(element->getMax()));
                defEditor.setText(juce::String(element->getDef()));
            }
        }
    }

    void inspectProject(PluginData::Project* project)
    {
        currentProject = project;
        currentElement = nullptr;

        updateVisibility();
        updateRoleOptions();

        if (project)
        {
            titleLabel.setText("AJUSTES DE PROYECTO", juce::dontSendNotification);

            projNameEditor.setText(project->pluginName);
            projManufEditor.setText(project->manufacturer);
            projURIEditor.setText(project->pluginURI);

            inputsCombo.setSelectedId(project->numInputs, juce::dontSendNotification);
            outputsCombo.setSelectedId(project->numOutputs, juce::dontSendNotification);
            algoCombo.setSelectedId((int)project->currentAlgorithm + 1, juce::dontSendNotification);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);

        titleLabel.setBounds(area.removeFromTop(30));
        area.removeFromTop(20);

        int h = 25, gap = 10;

        auto layoutField = [&](juce::Component& l, juce::Component& e)
        {
            if (e.isVisible())
            {
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

        if (algoCombo.isVisible())
        {
            layoutField(inputsLabel, inputsCombo);
            layoutField(outputsLabel, outputsCombo);
            layoutField(algoLabel, algoCombo);
        }
    }

    void textEditorFocusLost(juce::TextEditor&) override { applyChanges(); }
    void textEditorReturnKeyPressed(juce::TextEditor&) override { applyChanges(); }
    void comboBoxChanged(juce::ComboBox*) override { applyChanges(); }

private:
    VisualElement* currentElement = nullptr;
    PluginData::Project* currentProject = nullptr;

    juce::Label titleLabel;

    juce::Label compNameLabel, compIDLabel, minLabel, maxLabel, defLabel;
    juce::TextEditor compNameEditor, compIDEditor, minEditor, maxEditor, defEditor;

    juce::Label roleLabel;
    juce::ComboBox roleCombo;

    juce::Label projNameLabel, projManufLabel, projURILabel;
    juce::TextEditor projNameEditor, projManufEditor, projURIEditor;

    juce::Label inputsLabel, outputsLabel;
    juce::ComboBox inputsCombo, outputsCombo;

    juce::Label algoLabel;
    juce::ComboBox algoCombo;

    // ================================
    // 🔥 NUEVO: FILTRO DE ROLES
    // ================================
    void updateRoleOptions()
    {
        roleCombo.clear();

        if (!currentProject) return;

        using namespace PluginData;

        switch (currentProject->currentAlgorithm)
        {
            case AlgorithmType::Gain:
                roleCombo.addItem("Gain", 1);
                break;

            case AlgorithmType::Distortion:
                roleCombo.addItem("Drive", 1);
                roleCombo.addItem("Mix", 2);
                roleCombo.addItem("Tone", 3);
                break;

            case AlgorithmType::Tremolo:
                roleCombo.addItem("Frequency", 1);
                roleCombo.addItem("Depth", 2);
                break;

            default:
                roleCombo.addItem("None", 1);
                roleCombo.addItem("Gain", 2);
                roleCombo.addItem("Drive", 3);
                roleCombo.addItem("Mix", 4);
                roleCombo.addItem("Tone", 5);
                roleCombo.addItem("Frequency", 6);
                roleCombo.addItem("Depth", 7);
                break;
        }
    }

    PluginData::ParamRole getRoleFromCombo(int id)
    {
        using namespace PluginData;

        if (!currentProject) return ParamRole::None;

        switch (currentProject->currentAlgorithm)
        {
            case AlgorithmType::Gain:
                return ParamRole::Gain;

            case AlgorithmType::Distortion:
                if (id == 1) return ParamRole::Drive;
                if (id == 2) return ParamRole::Mix;
                if (id == 3) return ParamRole::Tone;
                break;

            case AlgorithmType::Tremolo:
                if (id == 1) return ParamRole::Frequency;
                if (id == 2) return ParamRole::Depth;
                break;

            default:
                return (ParamRole)(id - 1);
        }

        return ParamRole::None;
    }

    void addCompField(juce::Label& l, juce::TextEditor& e, const juce::String& t)
    {
        l.setText(t, juce::dontSendNotification);
        addAndMakeVisible(l);
        addAndMakeVisible(e);
        e.addListener(this);
    }

    void addProjField(juce::Label& l, juce::TextEditor& e, const juce::String& t)
    {
        l.setText(t, juce::dontSendNotification);
        addAndMakeVisible(l);
        addAndMakeVisible(e);
        e.addListener(this);
    }

    bool isSliderLike() const
    {
        if (!currentElement) return false;

        return (currentElement->getType() == PluginData::ComponentType::Slider ||
                currentElement->getType() == PluginData::ComponentType::Knob);
    }

    void updateVisibility()
    {
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

    void applyChanges()
    {
        if (currentElement)
        {
            currentElement->setName(compNameEditor.getText());
            currentElement->setSymbol(compIDEditor.getText());

            if (isSliderLike())
            {
                currentElement->setRange(
                    minEditor.getText().getFloatValue(),
                    maxEditor.getText().getFloatValue(),
                    defEditor.getText().getFloatValue()
                );

                int roleId = roleCombo.getSelectedId();
                if (roleId > 0)
                    currentElement->setRole(getRoleFromCombo(roleId));
            }

            if (onDataChanged) onDataChanged();
        }
        else if (currentProject)
        {
            currentProject->pluginName = projNameEditor.getText();
            currentProject->manufacturer = projManufEditor.getText();
            currentProject->pluginURI = projURIEditor.getText();

            if (inputsCombo.getSelectedId() > 0)
                currentProject->numInputs = inputsCombo.getSelectedId();

            if (outputsCombo.getSelectedId() > 0)
                currentProject->numOutputs = outputsCombo.getSelectedId();

            if (algoCombo.getSelectedId() > 0)
            {
                currentProject->currentAlgorithm =
                    (PluginData::AlgorithmType)(algoCombo.getSelectedId() - 1);

                updateRoleOptions();
            }
        }
    }
};