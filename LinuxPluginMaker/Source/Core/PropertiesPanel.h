/*
  ==============================================================================
    Source/Core/PropertiesPanel.h
    Actualizado con Selector de Algoritmo y Canales de Audio
  ==============================================================================
*/
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

        // --- SECCIÓN COMPONENTE ---
        addCompField(compNameLabel, compNameEditor, "Nombre:");
        addCompField(compIDLabel, compIDEditor, "ID (Symbol):");
        addCompField(minLabel, minEditor, "Mínimo:");
        addCompField(maxLabel, maxEditor, "Máximo:");
        addCompField(defLabel, defEditor, "Defecto:");

        // --- SECCIÓN PROYECTO ---
        addProjField(projNameLabel, projNameEditor, "Nombre Plugin:");
        addProjField(projManufLabel, projManufEditor, "Fabricante:");
        addProjField(projURILabel, projURIEditor, "URI Único:");
        
        // --- NUEVO: SELECTORES DE CANALES DE AUDIO ---
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

        // --- SELECTOR DE ALGORITMO ---
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
        currentProject = nullptr;
        
        updateVisibility();

        if (element)
        {
            titleLabel.setText("EDITAR COMPONENTE", juce::dontSendNotification);
            compNameEditor.setText(element->getName());
            compIDEditor.setText(element->getSymbol());

            bool isSlider = (element->getType() == PluginData::ComponentType::Slider || 
                             element->getType() == PluginData::ComponentType::Knob);
            
            minEditor.setVisible(isSlider); minLabel.setVisible(isSlider);
            maxEditor.setVisible(isSlider); maxLabel.setVisible(isSlider);
            defEditor.setVisible(isSlider); defLabel.setVisible(isSlider);

            if (isSlider) {
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

        if (project)
        {
            titleLabel.setText("AJUSTES DE PROYECTO", juce::dontSendNotification);
            projNameEditor.setText(project->pluginName);
            projManufEditor.setText(project->manufacturer);
            projURIEditor.setText(project->pluginURI);
            
            // --- NUEVO: Cargar valores en los menús desplegables ---
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

        int h = 25; int gap = 10;

        auto layoutField = [&](juce::Component& l, juce::Component& e)
        {
            if (e.isVisible()) {
                auto row = area.removeFromTop(h);
                l.setBounds(row.removeFromLeft(90)); 
                e.setBounds(row);                    
                area.removeFromTop(gap);
            }
        };

        layoutField(compNameLabel, compNameEditor);
        layoutField(compIDLabel, compIDEditor);
        area.removeFromTop(5); 
        layoutField(minLabel, minEditor);
        layoutField(maxLabel, maxEditor);
        layoutField(defLabel, defEditor);

        layoutField(projNameLabel, projNameEditor);
        layoutField(projManufLabel, projManufEditor);
        layoutField(projURILabel, projURIEditor);
        
        // --- NUEVO: Layout de los desplegables de Proyecto ---
        if (algoCombo.isVisible()) {
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

    juce::Label projNameLabel, projManufLabel, projURILabel;
    juce::TextEditor projNameEditor, projManufEditor, projURIEditor;
    
    // --- NUEVO: Variables para los combos de audio ---
    juce::Label inputsLabel, outputsLabel;
    juce::ComboBox inputsCombo, outputsCombo;

    juce::Label algoLabel;
    juce::ComboBox algoCombo;

    void addCompField(juce::Label& l, juce::TextEditor& e, const juce::String& t) {
        l.setText(t, juce::dontSendNotification); addAndMakeVisible(l);
        addAndMakeVisible(e); e.addListener(this);
    }
    void addProjField(juce::Label& l, juce::TextEditor& e, const juce::String& t) {
        l.setText(t, juce::dontSendNotification); addAndMakeVisible(l);
        addAndMakeVisible(e); e.addListener(this);
    }

    void updateVisibility()
    {
        bool showComp = (currentElement != nullptr);
        bool showProj = (currentProject != nullptr);

        compNameLabel.setVisible(showComp); compNameEditor.setVisible(showComp);
        compIDLabel.setVisible(showComp); compIDEditor.setVisible(showComp);
        if (!showComp) {
            minLabel.setVisible(false); minEditor.setVisible(false);
            maxLabel.setVisible(false); maxEditor.setVisible(false);
            defLabel.setVisible(false); defEditor.setVisible(false);
        }

        projNameLabel.setVisible(showProj); projNameEditor.setVisible(showProj);
        projManufLabel.setVisible(showProj); projManufEditor.setVisible(showProj);
        projURILabel.setVisible(showProj); projURIEditor.setVisible(showProj);
        
        // --- NUEVO: Mostrar/Ocultar combos de audio ---
        inputsLabel.setVisible(showProj); inputsCombo.setVisible(showProj);
        outputsLabel.setVisible(showProj); outputsCombo.setVisible(showProj);

        algoLabel.setVisible(showProj); algoCombo.setVisible(showProj);

        resized(); 
    }

    void applyChanges()
    {
        if (currentElement)
        {
            currentElement->setName(compNameEditor.getText());
            currentElement->setSymbol(compIDEditor.getText());
            if (currentElement->getType() == PluginData::ComponentType::Slider || 
                currentElement->getType() == PluginData::ComponentType::Knob) {
                currentElement->setRange(
                    minEditor.getText().getFloatValue(),
                    maxEditor.getText().getFloatValue(),
                    defEditor.getText().getFloatValue()
                );
            }
            if (onDataChanged) onDataChanged();
        }
        else if (currentProject)
        {
            currentProject->pluginName = projNameEditor.getText();
            currentProject->manufacturer = projManufEditor.getText();
            currentProject->pluginURI = projURIEditor.getText();
            
            // --- NUEVO: Guardar canales en el proyecto ---
            int inId = inputsCombo.getSelectedId();
            if (inId > 0) currentProject->numInputs = inId;

            int outId = outputsCombo.getSelectedId();
            if (outId > 0) currentProject->numOutputs = outId;

            int id = algoCombo.getSelectedId();
            if (id > 0) currentProject->currentAlgorithm = (PluginData::AlgorithmType)(id - 1);
        }
    }
};