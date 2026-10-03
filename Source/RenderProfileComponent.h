/*
  ==============================================================================

    RenderProfileComponent.h
    Created: 25 Nov 2025 12:53:44pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/

class RenderProfileComponent  : public juce::Component
{
public:
    RenderProfileComponent(int id);
    ~RenderProfileComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void addComponent(juce::Component* component) {
        addAndMakeVisible(*component);
        components.push_back(component);
    }

    int getRenderStateID() {
        return renderStateID;
    }

    void setPresetName(const juce::String& newName) {
        name = newName;

        if (auto* preset = frontEndPresets.getDynamicObject())
            preset->setProperty("value", name);
    }

    juce::String getPresetName() const {
        return name;
    }

    void setResizableBounds(juce::Rectangle<int> newBounds) {
        bounds = newBounds;
        setBounds(newBounds);
    }

    void show() {
        for (auto component : components) {
            addAndMakeVisible(component);
        }
        setBounds(bounds);
    }

    void hide() {
        for (auto component : components) {
            removeChildComponent(component);
        }
    }

    juce::var getFrontEndPresets() const {
        return frontEndPresets;
    }

    void setFrontEndPresets(const juce::var& newPresets) {
        frontEndPresets = newPresets;
    }

private:

    juce::var frontEndPresets;

    int renderStateID;
    juce::Rectangle<int> bounds;
    juce::String name = "Default";

    std::vector<juce::Component*> components;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderProfileComponent)
};
