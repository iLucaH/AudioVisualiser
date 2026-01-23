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
    presetBox.setHelpText("Click to change render states!");
    presetBox.setBounds(0, 0, 50, 50);
    presetBox.onClick = [this]() {
        int newState = 1 + ((selectedState) % (openGLComponent.getNumRenderStates()));
        updatePanelRenderProfile(newState, selectedState);
        selectedState = newState;
        openGLComponent.setSelectedState(selectedState);
    };
    addAndMakeVisible(presetBox);

    for (int i = 0; i < openGL.getNumRenderStates(); i++) {
        addRenderPofile(openGL.getProfileComponent(i));
    }
}

SelectorTabPanel::~SelectorTabPanel(){
}

void SelectorTabPanel::paint (juce::Graphics& g) {
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::white);
    g.drawRect (getLocalBounds(), 1);
    g.drawText(getCurrentRenderProfile()->getPresetName(), 0, 0, 100, 100, juce::Justification::topLeft, false);
}

void SelectorTabPanel::resized() {
    auto area = getLocalBounds();
    for (auto* profile : renderProfiles)
        profile->setBounds(juce::Rectangle(0, 168, 140, 298));
}