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
#include "PluginProcessor.h"
#include "SettingsComponent.h"
#include "Settings.h"
#include "AppQRComponent.h"

//==============================================================================
/*
*/

#define DEFAULT_RENDER_STATE 1

class SelectorTabPanel : public juce::Component {
public:
    SelectorTabPanel(AudioVisualiserAudioProcessor&, OpenGLComponent&, ApplicationSettings&);
    
    ~SelectorTabPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void processPlay();
    void processStop();
    void processRenderStateIncrement();
    void processRenderStateDecrement();

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
    AudioVisualiserAudioProcessor& pluginProcessor;
    OpenGLComponent& openGLComponent;

    ApplicationSettings& appSettings;

    AppQRComponent appQRComponent;
    SettingsComponent settingsComponent;

    juce::ComboBox presetSelector; //, fftSelector;
    juce::TextButton open, play, stop, settings, openInApp, fullscreen;
    juce::FileChooser openChooser;

    unsigned int selectedState = 1;
    std::vector<RenderProfileComponent*> renderProfiles;
    unsigned int fftSampleRate = 11;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelectorTabPanel)
};