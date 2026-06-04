#pragma once
#include "PluginData.h"
#include <juce_gui_basics/juce_gui_basics.h>

class VisualElement : public juce::Component {
public:
  std::function<void()> onClick;

  VisualElement(PluginData::ComponentType type, int index,
                const juce::String &name)
      : componentType(type), componentIndex(index) {
    setOpaque(false);

    // Datos internos que despues se guardan en el proyecto y se traducen a LV2.
    compName = name;

    compSymbol =
        (type == PluginData::ComponentType::Slider ? "control" : "switch") +
        juce::String(index);

    if (type == PluginData::ComponentType::Slider ||
        type == PluginData::ComponentType::Knob) {
      paramName = name;
    } else if (type == PluginData::ComponentType::Selector) {
      paramName = "Mode";
    } else {
      paramName = name;
    }

    // Se mantiene para poder abrir proyectos antiguos basados en roles fijos.
    role = PluginData::ParamRole::None;

    if (type == PluginData::ComponentType::Slider ||
        type == PluginData::ComponentType::Knob ||
        type == PluginData::ComponentType::Selector) {
      minVal = 0.0f;
      maxVal = 1.0f;
      defVal = 0.0f;
    } else {
      minVal = 0.0f;
      maxVal = 1.0f;
      defVal = 0.0f;
    }

    // Representacion visual usada dentro del lienzo del IDE.
    if (type == PluginData::ComponentType::Slider ||
        type == PluginData::ComponentType::Knob ||
        type == PluginData::ComponentType::Selector) {

      slider.reset(new juce::Slider());

      if (type == PluginData::ComponentType::Slider)
        slider->setSliderStyle(juce::Slider::LinearVertical);
      else if (type == PluginData::ComponentType::Selector)
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
      else
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

      slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
      slider->setInterceptsMouseClicks(false, false);

      addAndMakeVisible(slider.get());

      label.setText(name, juce::dontSendNotification);
      label.setJustificationType(juce::Justification::centred);
      label.attachToComponent(slider.get(), false);

      if (type == PluginData::ComponentType::Selector)
        setSize(100, 70);
      else
        setSize(80, 100);
    } else if (type == PluginData::ComponentType::Toggle) {
      toggle.reset(new juce::ToggleButton(name));
      toggle->setInterceptsMouseClicks(false, false);

      addAndMakeVisible(toggle.get());
      setSize(100, 30);
    }
  }

  void setName(const juce::String &n) {
    compName = n;
    label.setText(n, juce::dontSendNotification);
    if (toggle)
      toggle->setButtonText(n);
    repaint();
  }
  juce::String getName() const { return compName; }

  void setSymbol(const juce::String &s) { compSymbol = s; }
  juce::String getSymbol() const { return compSymbol; }

  void setRange(float mn, float mx, float df) {
    minVal = mn;
    maxVal = mx;
    defVal = df;
  }
  float getMin() const { return minVal; }
  float getMax() const { return maxVal; }
  float getDef() const { return defVal; }

  // Numero de estados disponibles para controles discretos.
  void setNumSteps(int s) { numSteps = s; }
  int getNumSteps() const { return numSteps; }

  // Nombre del parametro DSP asociado a este control.
  void setParamName(const juce::String &name) {
    paramName = name;
    repaint();
  }
  juce::String getParamName() const { return paramName; }

  // Rol antiguo conservado por compatibilidad.
  void setRole(PluginData::ParamRole r) { role = r; }
  PluginData::ParamRole getRole() const { return role; }

  // Marca visualmente el componente seleccionado en el lienzo.
  void setSelected(bool shouldBeSelected) {
    isSelected = shouldBeSelected;
    repaint();
  }
  bool getSelected() const { return isSelected; }

  // Selecciona el componente y permite arrastrarlo por el lienzo.
  void mouseDown(const juce::MouseEvent &e) override {
    if (onClick)
      onClick();

    dragger.startDraggingComponent(this, e);
    toFront(true);
    repaint();
  }

  void mouseDrag(const juce::MouseEvent &e) override {
    dragger.dragComponent(this, e, nullptr);

    const int gridSize = 20;

    int x = (getX() / gridSize) * gridSize;
    int y = (getY() / gridSize) * gridSize;

    setTopLeftPosition(x, y);

    if (getX() < 0)
      setTopLeftPosition(0, getY());
    if (getY() < 0)
      setTopLeftPosition(getX(), 0);
  }

  // Dibuja el marco de seleccion y el nombre de parametro asociado.
  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colours::white.withAlpha(0.05f));

    if (isSelected) {
      g.setColour(juce::Colours::lightgreen);
      g.drawRect(getLocalBounds(), 2);
    } else {
      g.setColour(juce::Colours::orange.withAlpha(0.5f));
      g.drawRect(getLocalBounds(), 1);
    }

    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(10.0f);
    g.drawText(paramName, getLocalBounds().removeFromBottom(15),
               juce::Justification::centred);
  }

  void resized() override {
    if (slider)
      slider->setBounds(0, 20, getWidth(), getHeight() - 20);
    if (toggle)
      toggle->setBounds(0, 0, getWidth(), getHeight());
  }

  PluginData::ComponentType getType() const { return componentType; }
  int getIndex() const { return componentIndex; }

private:
  PluginData::ComponentType componentType;
  int componentIndex;

  // Datos persistentes del componente.
  juce::String compName;
  juce::String compSymbol;
  float minVal, maxVal, defVal;
  bool isSelected = false;

  // Parametro DSP mostrado en la parte inferior del control.
  juce::String paramName;

  // Numero de opciones para controles discretos.
  int numSteps = 3;

  // Rol antiguo conservado por compatibilidad.
  PluginData::ParamRole role = PluginData::ParamRole::None;

  // Componentes JUCE usados para representar el control en el IDE.
  std::unique_ptr<juce::Slider> slider;
  std::unique_ptr<juce::ToggleButton> toggle;
  juce::Label label;

  juce::ComponentDragger dragger;
};
