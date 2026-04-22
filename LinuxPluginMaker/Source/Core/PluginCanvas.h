#pragma once
#include "VisualElement.h"
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class PluginCanvas : public juce::Component {
public:
  std::function<void(VisualElement *)> onSelectionChanged;

  PluginCanvas() {}

  // 🔥🔥🔥 NUEVO: referencia al Project (clave para dinámica)
  void setProject(PluginData::Project *proj) { currentProject = proj; }

  void addElement(PluginData::ComponentType type, int index,
                  const juce::String &name) {
    auto *newComp = new VisualElement(type, index, name);

    newComp->onClick = [this, newComp]() {
      deselectAll();
      newComp->setSelected(true);
      if (onSelectionChanged)
        onSelectionChanged(newComp);
    };

    const int itemWidth = 90;
    const int itemHeight = 110;
    const int margin = 20;

    int availableWidth = getWidth();
    if (availableWidth <= 0)
      availableWidth = 600;

    int itemsPerRow = (availableWidth - margin) / itemWidth;
    if (itemsPerRow < 1)
      itemsPerRow = 1;

    int currentCount = elements.size();
    int row = currentCount / itemsPerRow;
    int col = currentCount % itemsPerRow;

    int x = margin + (col * itemWidth);
    int y = margin + (row * itemHeight);

    newComp->setTopLeftPosition(x, y);
    addAndMakeVisible(newComp);

    elements.add(newComp);
    autoAssignParamNames();
  }

  void clearAll() { elements.clear(); }

  void deleteSelected() {
    for (int i = elements.size() - 1; i >= 0; --i) {
      if (elements[i]->getSelected()) {
        removeChildComponent(elements[i]);
        elements.remove(i);
      }
    }

    // limpiar selección en panel
    if (onSelectionChanged)
      onSelectionChanged(nullptr);

    repaint();
  }

  void loadProject(const PluginData::Project &proj) {
    clearAll();

    const int itemWidth = 90;
    const int itemHeight = 110;
    const int margin = 20;
    int availableWidth = getWidth() > 0 ? getWidth() : 600;
    int itemsPerRow = std::max(1, (availableWidth - margin) / itemWidth);

    for (const auto &c : proj.components) {
      auto *newComp = new VisualElement(c.type, c.index, c.name);
      newComp->setParamName(c.paramName); // 🔥🔥🔥 CLAVE
      newComp->setSymbol(c.symbol);
      newComp->setRange(c.min, c.max, c.def);

      // 🔥🔥🔥 NUEVO: cargar selector (opciones)
      if (c.type == PluginData::ComponentType::Selector) {
        newComp->setNumSteps(c.numSteps);
      }

      newComp->onClick = [this, newComp]() {
        deselectAll();
        newComp->setSelected(true);
        if (onSelectionChanged)
          onSelectionChanged(newComp);
      };

      int currentCount = elements.size();
      int row = currentCount / itemsPerRow;
      int col = currentCount % itemsPerRow;
      newComp->setTopLeftPosition(margin + (col * itemWidth),
                                  margin + (row * itemHeight));
      newComp->setRole(c.role);

      addAndMakeVisible(newComp);
      elements.add(newComp);
    }
    repaint();
    autoAssignParamNames();
  }

  void updateProjectData(PluginData::Project &project) {
    project.components.clear();

    for (auto *el : elements) {
      PluginData::Component comp;
      comp.index = el->getIndex();
      comp.type = el->getType();

      comp.name = el->getName();
      comp.symbol = el->getSymbol();
      comp.paramName = el->getParamName();

      if (comp.type == PluginData::ComponentType::Slider ||
          comp.type == PluginData::ComponentType::Knob) {
        comp.min = el->getMin();
        comp.max = el->getMax();
        comp.def = el->getDef();
      } else {
        comp.min = 0;
        comp.max = 1;
        comp.def = 0;
      }

      // 🔥🔥🔥 SELECTOR DINÁMICO DESDE XML
      if (comp.type == PluginData::ComponentType::Selector && currentProject) {
        int idx = currentProject->currentEffectIndex;

        if (idx >= 0 && idx < currentProject->availableEffects.size()) {
          auto &effect = currentProject->availableEffects[idx];

          if (!effect.selectors.empty()) {
            auto &sel = effect.selectors[0]; // 🔥 primer selector

            comp.options = sel.options;
            comp.numSteps = (int)sel.options.size();
          }
        }
      }

      comp.role = el->getRole();
      project.components.push_back(comp);
    }
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    for (int x = 0; x < getWidth(); x += 20)
      g.drawVerticalLine(x, 0.0f, (float)getHeight());
    for (int y = 0; y < getHeight(); y += 20)
      g.drawHorizontalLine(y, 0.0f, (float)getWidth());
  }

  void mouseDown(const juce::MouseEvent &e) override {
    deselectAll();
    if (onSelectionChanged)
      onSelectionChanged(nullptr);
  }

  void refreshAutoParams() { autoAssignParamNames(); }

private:
  juce::OwnedArray<VisualElement> elements;

  // 🔥🔥🔥 NUEVO
  PluginData::Project *currentProject = nullptr;

  void deselectAll() {
    for (auto *el : elements)
      el->setSelected(false);
  }

  void autoAssignParamNames() {
    if (!currentProject)
      return;

    int idx = currentProject->currentEffectIndex;

    if (idx < 0 || idx >= currentProject->availableEffects.size())
      return;

    auto &effect = currentProject->availableEffects[idx];

    int paramIndex = 0;

    for (auto *el : elements) {
      // SOLO sliders/knobs
      if (el->getType() == PluginData::ComponentType::Slider ||
          el->getType() == PluginData::ComponentType::Knob) {
        if (paramIndex < effect.params.size()) {
          el->setParamName(effect.params[paramIndex].name);
          paramIndex++;
        }
      }

      // SELECTOR → SIEMPRE MODE
      if (el->getType() == PluginData::ComponentType::Selector) {
        el->setParamName("Mode");
      }
    }

    repaint();
  }
};
