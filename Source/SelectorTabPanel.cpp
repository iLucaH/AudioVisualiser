/*
  ==============================================================================

    SelectorTabPanel.cpp
    Created: 19 Nov 2025 8:24:33am
    Author:  lucas

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SelectorTabPanel.h"
#include "RenderState2D.h"

//==============================================================================
SelectorTabPanel::SelectorTabPanel(OpenGLComponent& openGL) : openGLComponent(openGL) {
    // Render state logic
    presetSelector.setHelpText("Click here to select a render state!");
    presetSelector.setTextWhenNothingSelected("Select Preset!");
    presetSelector.setBounds(8, 8, 123, 25);
    presetSelector.onChange = [this]() {
        int newState = presetSelector.getSelectedId();
        updatePanelRenderProfile(newState, selectedState);
        selectedState = newState;
        openGLComponent.setSelectedState(selectedState);
        };
    addAndMakeVisible(&presetSelector);
    for (int i = 0; i < openGL.getNumRenderStates(); i++) {
        addRenderPofile(openGL.getProfileComponent(i));
    }
    presetSelector.setSelectedId(DEFAULT_RENDER_STATE);

    // Sizing logic
    setWidth.setText("1920");
    setWidth.setBounds(8, 58, 55, 25);
    setWidth.onReturnKey = [this]() {
        juce::String str = setWidth.getText();
        int i;
        bool err = false;
        try {
            i = std::stoi(str.toStdString());
        } catch (const std::invalid_argument& e) {
            err = true;
            return;
        } catch (const std::out_of_range& e) {
            err = true;
        }
        if (i < MIN_WIDTH || i > MAX_WIDTH || i % 2 != 0) {
            err = true;
        }
        if (err) {
            setWidth.setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::red);
        } else {
            setWidth.setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::darkslategrey);
            // handle dimention update here
        }
        repaint();
    };
    addAndMakeVisible(&setWidth);

    setHeight.setText("1080");
    setHeight.setBounds(76, 58, 55, 25);
    setHeight.onReturnKey = [this]() {
        juce::String str = setHeight.getText();
        int i;
        bool err = false;
        try {
            i = std::stoi(str.toStdString());
        }
        catch (const std::invalid_argument& e) {
            err = true;
            return;
        }
        catch (const std::out_of_range& e) {
            err = true;
        }
        if (i < MIN_HEIGHT || i > MAX_HEIGHT || i % 2 != 0) {
            err = true;
        }
        if (err) {
            setHeight.setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::red);
        }
        else {
            setHeight.setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::darkslategrey);
            // handle dimention update here
        }
        repaint();
        };
    addAndMakeVisible(&setHeight);
}

SelectorTabPanel::~SelectorTabPanel(){
}

void SelectorTabPanel::paint (juce::Graphics& g) {
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::white);
    g.drawRect (getLocalBounds(), 1);
    g.drawSingleLineText("Width", 8, 53);
    g.drawSingleLineText("Height", 76, 53);
}

void SelectorTabPanel::resized() {
    auto area = getLocalBounds();
    for (auto* profile : renderProfiles)
        profile->setBounds(juce::Rectangle(0, 168, 140, 298));
}