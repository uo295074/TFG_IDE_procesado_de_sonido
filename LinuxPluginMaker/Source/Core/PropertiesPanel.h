/*
  ==============================================================================
    Source/Core/PropertiesPanel.h
    Panel inteligente: Muestra propiedades del Componente O del Proyecto.
  ==============================================================================
*/
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "VisualElement.h"
#include "PluginData.h"

class PropertiesPanel : public juce::Component, 
                        public juce::TextEditor::Listener
{
public:
    std::function<void()> onDataChanged;

    PropertiesPanel()
    {
        // Título principal
        titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(titleLabel);

        // --- SECCIÓN A: PROPIEDADES DE COMPONENTE (Slider/Switch) ---
        addCompField(compNameLabel, compNameEditor, "Nombre:");
        addCompField(compIDLabel, compIDEditor, "ID (Symbol):");
        addCompField(minLabel, minEditor, "Mínimo:");
        addCompField(maxLabel, maxEditor, "Máximo:");
        addCompField(defLabel, defEditor, "Defecto:");

        // --- SECCIÓN B: PROPIEDADES DE PROYECTO (Globales) ---
        addProjField(projNameLabel, projNameEditor, "Nombre Plugin:");
        addProjField(projManufLabel, projManufEditor, "Fabricante:");
        addProjField(projURILabel, projURIEditor, "URI Único:");
    }

    // MODO 1: Editar un Slider/Switch
    void inspectElement(VisualElement* element)
    {
        currentElement = element;
        currentProject = nullptr; // Dejamos de editar el proyecto
        
        updateVisibility(); // Magia para ocultar/mostrar campos

        if (element)
        {
            titleLabel.setText("EDITAR COMPONENTE", juce::dontSendNotification);
            
            compNameEditor.setText(element->getName());
            compIDEditor.setText(element->getSymbol());

            bool isSlider = (element->getType() == PluginData::ComponentType::Slider);
            
            // Solo mostramos números si es slider
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

    // MODO 2: Editar Datos del Proyecto (RU-01)
    void inspectProject(PluginData::Project* project)
    {
        currentProject = project;
        currentElement = nullptr; // Dejamos de editar componentes

        updateVisibility();

        if (project)
        {
            titleLabel.setText("AJUSTES DE PROYECTO", juce::dontSendNotification);
            projNameEditor.setText(project->pluginName);
            projManufEditor.setText(project->manufacturer);
            projURIEditor.setText(project->pluginURI);
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
                l.setBounds(row.removeFromLeft(90)); // Etiqueta
                e.setBounds(row);                    // Editor
                area.removeFromTop(gap);
            }
        };

        // Maquetar Componentes
        layoutField(compNameLabel, compNameEditor);
        layoutField(compIDLabel, compIDEditor);
        area.removeFromTop(5); 
        layoutField(minLabel, minEditor);
        layoutField(maxLabel, maxEditor);
        layoutField(defLabel, defEditor);

        // Maquetar Proyecto
        layoutField(projNameLabel, projNameEditor);
        layoutField(projManufLabel, projManufEditor);
        layoutField(projURILabel, projURIEditor);
    }

    // Guardar cambios al pulsar enter o perder foco
    void textEditorFocusLost(juce::TextEditor&) override { applyChanges(); }
    void textEditorReturnKeyPressed(juce::TextEditor&) override { applyChanges(); }

private:
    VisualElement* currentElement = nullptr;
    PluginData::Project* currentProject = nullptr;

    juce::Label titleLabel;

    // Campos Componente
    juce::Label compNameLabel, compIDLabel, minLabel, maxLabel, defLabel;
    juce::TextEditor compNameEditor, compIDEditor, minEditor, maxEditor, defEditor;

    // Campos Proyecto
    juce::Label projNameLabel, projManufLabel, projURILabel;
    juce::TextEditor projNameEditor, projManufEditor, projURIEditor;

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

        // Grupo Componente
        compNameLabel.setVisible(showComp); compNameEditor.setVisible(showComp);
        compIDLabel.setVisible(showComp); compIDEditor.setVisible(showComp);
        // (Los numéricos se gestionan dentro de inspectElement porque dependen de si es Slider o Switch)
        if (!showComp) {
            minLabel.setVisible(false); minEditor.setVisible(false);
            maxLabel.setVisible(false); maxEditor.setVisible(false);
            defLabel.setVisible(false); defEditor.setVisible(false);
        }

        // Grupo Proyecto
        projNameLabel.setVisible(showProj); projNameEditor.setVisible(showProj);
        projManufLabel.setVisible(showProj); projManufEditor.setVisible(showProj);
        projURILabel.setVisible(showProj); projURIEditor.setVisible(showProj);

        resized(); // Recalcular layout
    }

    void applyChanges()
    {
        if (currentElement)
        {
            currentElement->setName(compNameEditor.getText());
            currentElement->setSymbol(compIDEditor.getText());
            if (currentElement->getType() == PluginData::ComponentType::Slider) {
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
        }
    }
};