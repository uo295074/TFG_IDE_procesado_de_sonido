/*
  ==============================================================================
    Source/Core/PluginCanvas.h
    El área de trabajo visual.
    Actualizado para gestionar la SELECCIÓN y comunicar clicks al MainComponent.
  ==============================================================================
*/
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "VisualElement.h"
#include <vector>
#include <functional> // Necesario para std::function

class PluginCanvas : public juce::Component
{
public:
    // --- ESTA ES LA VARIABLE QUE FALTABA ---
    // Es el "cable" que conecta este Canvas con el MainComponent
    std::function<void(VisualElement*)> onSelectionChanged;

    PluginCanvas()
    {
    }

    void addElement(PluginData::ComponentType type, int index, const juce::String& name)
    {
        // Crear nuevo elemento visual
        auto* newComp = new VisualElement(type, index, name);
        
        // --- CONECTAR EL CLIC DEL ELEMENTO AL CANVAS ---
        // Cuando el usuario haga clic en el VisualElement, este avisará aquí
        newComp->onClick = [this, newComp]()
        {
            // 1. Deseleccionamos todo lo demás visualmente
            deselectAll();
            
            // 2. Marcamos este como seleccionado
            newComp->setSelected(true);
            
            // 3. Avisamos al MainComponent (para que actualice el panel lateral)
            if (onSelectionChanged) onSelectionChanged(newComp);
        };

        // --- CÁLCULO DE REJILLA (GRID) ---
        const int itemWidth = 90;
        const int itemHeight = 110;
        const int margin = 20;

        int availableWidth = getWidth();
        if (availableWidth <= 0) availableWidth = 600; 
        
        int itemsPerRow = (availableWidth - margin) / itemWidth;
        if (itemsPerRow < 1) itemsPerRow = 1;

        int currentCount = elements.size();
        int row = currentCount / itemsPerRow;
        int col = currentCount % itemsPerRow;

        int x = margin + (col * itemWidth);
        int y = margin + (row * itemHeight);
        
        newComp->setTopLeftPosition(x, y);
        addAndMakeVisible(newComp);
        
        // Guardar en nuestro vector
        elements.add(newComp);
    }

    void clearAll()
    {
        elements.clear(); // Esto borra y destruye los componentes automáticamente
    }

    // --- RU-02: RECONSTRUIR UI DESDE DATOS CARGADOS ---
    void loadProject(const PluginData::Project& proj)
    {
        clearAll(); // Borramos lo actual

        const int itemWidth = 90;
        const int itemHeight = 110;
        const int margin = 20;
        int availableWidth = getWidth() > 0 ? getWidth() : 600; 
        int itemsPerRow = std::max(1, (availableWidth - margin) / itemWidth);

        for (const auto& c : proj.components)
        {
            // Recreamos el elemento visual
            auto* newComp = new VisualElement(c.type, c.index, c.name);
            newComp->setSymbol(c.symbol);
            newComp->setRange(c.min, c.max, c.def);
            
            // Le volvemos a conectar el "cable" del clic
            newComp->onClick = [this, newComp]() {
                deselectAll();
                newComp->setSelected(true);
                if (onSelectionChanged) onSelectionChanged(newComp);
            };

            // Calculamos su posición en la rejilla
            int currentCount = elements.size();
            int row = currentCount / itemsPerRow;
            int col = currentCount % itemsPerRow;
            newComp->setTopLeftPosition(margin + (col * itemWidth), margin + (row * itemHeight));
            newComp->setRole(c.role);
            addAndMakeVisible(newComp);
            elements.add(newComp);
        }
        repaint();
    }
    
    // Método para sincronizar con PluginData antes de generar
   void updateProjectData(PluginData::Project& project)
    {
        project.components.clear();
        
        for (auto* el : elements)
        {
            PluginData::Component comp;
            comp.index = el->getIndex();
            comp.type = el->getType();
            
            // Recuperamos los datos editados en el panel
            comp.name = el->getName();
            comp.symbol = el->getSymbol();
            
            // --- CAMBIO AQUÍ: El Knob también usa min, max y def ---
            if (comp.type == PluginData::ComponentType::Slider || 
                comp.type == PluginData::ComponentType::Knob)
            {
                comp.min = el->getMin();
                comp.max = el->getMax();
                comp.def = el->getDef();
            }
            else // Para el Toggle (Switch)
            {
                comp.min = 0; comp.max = 1; comp.def = 0;
            }
            comp.role = el->getRole();
            project.components.push_back(comp);
        }
    }

    void paint(juce::Graphics& g) override
    {
        // Fondo negro con rejilla
        g.fillAll(juce::Colours::black);
        
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        for (int x = 0; x < getWidth(); x += 20) g.drawVerticalLine(x, 0.0f, (float)getHeight());
        for (int y = 0; y < getHeight(); y += 20) g.drawHorizontalLine(y, 0.0f, (float)getWidth());
    }
    
    // --- GESTIÓN DE CLIC EN EL FONDO VACÍO ---
    void mouseDown(const juce::MouseEvent& e) override
    {
        // Si clicas en el negro, se deselecciona todo
        deselectAll();
        
        // Avisamos con 'nullptr' para que el panel se oculte
        if (onSelectionChanged) onSelectionChanged(nullptr);
    }

private:
    juce::OwnedArray<VisualElement> elements;

    void deselectAll()
    {
        for (auto* el : elements)
            el->setSelected(false);
    }
};