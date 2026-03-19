/*
  ==============================================================================
    Source/Core/CodeEditorPanel.h
    Panel para editar el código C++ del DSP.
  ==============================================================================
*/
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginData.h"

class CodeEditorPanel : public juce::Component
{
public:
    CodeEditorPanel(PluginData::Project& p) : project(p)
    {
        // Cargar el código en el documento
        codeDocument.replaceAllContent(project.customDspCode);
        // Fondo
        editor.setColour(juce::CodeEditorComponent::backgroundColourId, juce::Colours::white);

        // Texto por defecto
        editor.setColour(juce::CodeEditorComponent::defaultTextColourId, juce::Colours::white);

        // Selección
        editor.setColour(juce::CodeEditorComponent::highlightColourId, juce::Colours::darkgrey);

        // Cursor
        editor.setColour(juce::CaretComponent::caretColourId, juce::Colours::white);

        // Números de línea
        editor.setColour(juce::CodeEditorComponent::lineNumberTextId, juce::Colours::grey);

        // Fondo de números de línea
        editor.setColour(juce::CodeEditorComponent::lineNumberBackgroundId, juce::Colours::black);


        addAndMakeVisible(editor);

        // Botón guardar
        saveBtn.setButtonText("Guardar y Cerrar");
        saveBtn.onClick = [this]
        {
            // Guardar contenido del editor
            project.customDspCode = codeDocument.getAllContent();

            // Cerrar ventana
            if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                dw->exitModalState(0);
        };

        addAndMakeVisible(saveBtn);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        saveBtn.setBounds(area.removeFromBottom(40).removeFromRight(150));
        area.removeFromBottom(10);
        editor.setBounds(area);
    }

private:
    PluginData::Project& project;

    juce::CodeDocument codeDocument;
    juce::CPlusPlusCodeTokeniser tokeniser;
    juce::CodeEditorComponent editor { codeDocument, &tokeniser };

    juce::TextButton saveBtn;
};