#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class CodeEditorPanel : public juce::Component {
public:
  // 🔥 Ahora recibe una referencia a cualquier string (DSP, INIT, etc.)
  CodeEditorPanel(juce::String &codeToEdit) : codeRef(codeToEdit) {
    // Cargar el código en el documento
    codeDocument.replaceAllContent(codeRef);

    // 🎨 COLORES (modo claro legible)
    editor.setColour(juce::CodeEditorComponent::backgroundColourId,
                     juce::Colours::white);
    editor.setColour(juce::CodeEditorComponent::defaultTextColourId,
                     juce::Colours::black);

    editor.setColour(juce::CodeEditorComponent::highlightColourId,
                     juce::Colours::lightblue.withAlpha(0.3f));

    editor.setColour(juce::CaretComponent::caretColourId, juce::Colours::black);

    // números de línea
    editor.setColour(juce::CodeEditorComponent::lineNumberTextId,
                     juce::Colours::darkgrey);
    editor.setColour(juce::CodeEditorComponent::lineNumberBackgroundId,
                     juce::Colours::lightgrey.withAlpha(0.3f));

    editor.setFont(juce::Font(14.0f));
    addAndMakeVisible(editor);

    // 🔥 BOTÓN GUARDAR
    saveBtn.setButtonText("Guardar y Cerrar");
    saveBtn.onClick = [this] {
      // Guardar contenido en la referencia
      codeRef = codeDocument.getAllContent();

      // Cerrar ventana
      if (auto *dw = findParentComponentOfClass<juce::DialogWindow>())
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
  // 🔥 CLAVE: referencia genérica al código
  juce::String &codeRef;

  juce::CodeDocument codeDocument;
  juce::CPlusPlusCodeTokeniser tokeniser;
  juce::CodeEditorComponent editor{codeDocument, &tokeniser};

  juce::TextButton saveBtn;
};