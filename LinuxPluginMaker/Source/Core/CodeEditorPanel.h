/*
  ==============================================================================
    Source/Core/CodeEditorPanel.h
    Panel para editar el código C++ del DSP.
  ==============================================================================
*/
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginData.h"

class CodeEditorPanel : public juce::Component
{
public:
    CodeEditorPanel(PluginData::Project& p) : project(p)
    {
        // Configurar el editor de texto como un editor de código
        editor.setMultiLine(true);
        editor.setReturnKeyStartsNewLine(true);
        editor.setTabKeyUsedAsCharacter(true); // ¡Importante para programar!
        editor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
        
        // Colores "oscuros" estilo Hacker
        editor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
        editor.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
        
        // Cargar el código actual
        editor.setText(project.customDspCode);
        
        addAndMakeVisible(editor);

        saveBtn.setButtonText("Guardar y Cerrar");
        saveBtn.onClick = [this] {
            project.customDspCode = editor.getText();
            // Cerramos la ventana (buscamos la DialogWindow que nos contiene)
            if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                dw->exitModalState(0);
        };
        addAndMakeVisible(saveBtn);
    }

    void resized() override {
        auto area = getLocalBounds().reduced(10);
        saveBtn.setBounds(area.removeFromBottom(40).removeFromRight(150));
        area.removeFromBottom(10);
        editor.setBounds(area);
    }

private:
    PluginData::Project& project;
    juce::TextEditor editor;
    juce::TextButton saveBtn;
};