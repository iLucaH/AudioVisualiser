/*
  ==============================================================================

    SelectorTabPanel.h
    Created: 19 Nov 2025 8:24:33am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "OpenGLComponent.h"
#include "RenderProfileComponent.h"

//==============================================================================
/*
*/

#define DEFAULT_RENDER_STATE 1
#define MIN_WIDTH 100
#define MAX_WIDTH 1920
#define MIN_HEIGHT 100
#define MAX_HEIGHT 1080

class SelectorTabPanel  : public juce::Component {
public:
    SelectorTabPanel(OpenGLComponent&);
    
    ~SelectorTabPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void addRenderPofile(RenderProfileComponent* component) {
        const int index = renderProfiles.size();
        component->setResizableBounds(juce::Rectangle(0, 168, 140, 298));
        addAndMakeVisible(component);
        if (component->getRenderStateID() != selectedState)
            component->setVisible(false);
        renderProfiles.push_back(component);
        presetSelector.addItem(component->getPresetName(), component->getRenderStateID());
    }

    void updatePanelRenderProfile(int newState, int oldState) {
        int new_id = newState - 1;
        int old_id = oldState - 1;

        if (old_id < 0 || old_id >= renderProfiles.size()) return;
        if (new_id < 0 || new_id >= renderProfiles.size()) return;

        auto* newComponent = renderProfiles[new_id];
        auto* oldComponent = renderProfiles[old_id];

        oldComponent->setVisible(false);
        newComponent->setVisible(true);

        resized();
    }

    RenderProfileComponent* getCurrentRenderProfile() {
        return renderProfiles[selectedState - 1];
    }

private:
    OpenGLComponent& openGLComponent;

    juce::ComboBox presetSelector, fftSelector;
    juce::TextEditor setWidth, setHeight;

    unsigned int selectedState = 1;
    std::vector<RenderProfileComponent*> renderProfiles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelectorTabPanel)
};