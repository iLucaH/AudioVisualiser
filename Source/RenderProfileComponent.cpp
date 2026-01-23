/*
  ==============================================================================

    RenderProfileComponent.cpp
    Created: 25 Nov 2025 12:53:44pm
    Author:  lucas

  ==============================================================================
*/

#include <JuceHeader.h>
#include "RenderProfileComponent.h"

//==============================================================================
RenderProfileComponent::RenderProfileComponent(int id) : renderStateID(id), bounds(0, 0, 0, 0) {}

RenderProfileComponent::~RenderProfileComponent() {}

void RenderProfileComponent::paint (juce::Graphics& g) {
    for (int i = 0; i < components.size(); i++) {
        Component* component = components[i];
    }
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds(), 1);   // draw an outline around the component
}

void RenderProfileComponent::resized() {}
