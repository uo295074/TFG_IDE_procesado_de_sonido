/*
  ==============================================================================
    Source/Core/VisualElement.h
    Representa un componente visual en el canvas.
    Actualizado para cumplir RU-05 (Propiedades editables) y RU-06 (Diseño visual).
  ==============================================================================
*/
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginData.h"

class VisualElement : public juce::Component
{
public:
    // Callback público: Avisa al padre cuando se hace clic en este elemento
    std::function<void()> onClick;

    VisualElement(PluginData::ComponentType type, int index, const juce::String& name)
        : componentType(type), componentIndex(index)
    {
        // Configuración visual del contenedor arrastrable
        setOpaque(false);
        
        // --- INICIALIZACIÓN DE DATOS (RU-05) ---
        compName = name;
        // Generamos un ID único por defecto (ej: control1, switch2)
        compSymbol = (type == PluginData::ComponentType::Slider ? "control" : "switch") + juce::String(index);
        
        // Valores por defecto
        if (type == PluginData::ComponentType::Slider) {
            minVal = 0.0f; maxVal = 1.0f; defVal = 0.5f;
        } else {
            minVal = 0.0f; maxVal = 1.0f; defVal = 0.0f; // Para switch 0 o 1
        }

        // --- CONSTRUCCIÓN DE LA INTERFAZ ---
        // 1. CREAR SLIDER
        if (type == PluginData::ComponentType::Slider)
        {
            slider.reset(new juce::Slider());
            slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
            
            // EL TRUCO: Desactivar interacción del hijo para permitir arrastre del padre
            slider->setInterceptsMouseClicks(false, false); 
            
            addAndMakeVisible(slider.get());
            
            // Etiqueta
            label.setText(name, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            label.attachToComponent(slider.get(), false);
            
            setSize(80, 100); 
        }
        // 2. CREAR SWITCH
        else if (type == PluginData::ComponentType::Toggle)
        {
            toggle.reset(new juce::ToggleButton(name));
            toggle->setInterceptsMouseClicks(false, false);
            
            addAndMakeVisible(toggle.get());
            setSize(100, 30);
        }
    }

    // --- MÉTODOS DE ACCESO PARA EL INSPECTOR DE PROPIEDADES ---
    void setName(const juce::String& n) 
    { 
        compName = n; 
        // Actualizamos también la etiqueta visual para que el usuario vea el cambio
        label.setText(n, juce::dontSendNotification);
        if (toggle) toggle->setButtonText(n);
        repaint();
    }
    juce::String getName() const { return compName; }

    void setSymbol(const juce::String& s) { compSymbol = s; }
    juce::String getSymbol() const { return compSymbol; }

    void setRange(float mn, float mx, float df) { minVal = mn; maxVal = mx; defVal = df; }
    float getMin() const { return minVal; }
    float getMax() const { return maxVal; }
    float getDef() const { return defVal; }

    // Gestión de Selección
    void setSelected(bool shouldBeSelected)
    {
        isSelected = shouldBeSelected;
        repaint(); // Forzar repintado para cambiar el color del borde
    }
    bool getSelected() const { return isSelected; }

    // --- EVENTOS DE RATÓN (ARRASTRE Y SELECCIÓN) ---
    void mouseDown(const juce::MouseEvent& e) override
    {
        // 1. Notificar selección
        if (onClick) onClick();

        // 2. Iniciar arrastre
        dragger.startDraggingComponent(this, e);
        toFront(true); 
        repaint();
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        dragger.dragComponent(this, e, nullptr);
        
        // Límites de seguridad para no perder el componente fuera de pantalla
        if (getX() < 0) setTopLeftPosition(0, getY());
        if (getY() < 0) setTopLeftPosition(getX(), 0);
    }

    void paint(juce::Graphics& g) override
    {
        // Fondo semitransparente para ver el área de agarre
        g.fillAll(juce::Colours::white.withAlpha(0.05f));
        
        // Borde indicador de selección
        if (isSelected)
        {
            g.setColour(juce::Colours::lightgreen); // Verde si está seleccionado
            g.drawRect(getLocalBounds(), 2);        // Borde más grueso
        }
        else
        {
            g.setColour(juce::Colours::orange.withAlpha(0.5f)); // Naranja tenue si no
            g.drawRect(getLocalBounds(), 1);
        }
    }

    void resized() override
    {
        if (slider) slider->setBounds(0, 20, getWidth(), getHeight() - 20);
        if (toggle) toggle->setBounds(0, 0, getWidth(), getHeight());
    }

    PluginData::ComponentType getType() const { return componentType; }
    int getIndex() const { return componentIndex; }

private:
    PluginData::ComponentType componentType;
    int componentIndex;
    
    // Datos Lógicos (Modelo)
    juce::String compName;
    juce::String compSymbol;
    float minVal, maxVal, defVal;
    bool isSelected = false;

    // Componentes Visuales (Vista)
    std::unique_ptr<juce::Slider> slider;
    std::unique_ptr<juce::ToggleButton> toggle;
    juce::Label label;
    
    juce::ComponentDragger dragger; 
};