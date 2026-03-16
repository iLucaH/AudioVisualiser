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
SelectorTabPanel::SelectorTabPanel(AudioVisualiserAudioProcessor& p, OpenGLComponent& openGL, ApplicationSettings& appSettings) : pluginProcessor(p), openGLComponent(openGL), settingsComponent(appSettings),
    openChooser("Choose a Wav or AIFF File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.mp3") {
    // Render state logic
    presetSelector.setHelpText("Click here to select a render state!");
    presetSelector.setTextWhenNothingSelected("Select Preset");
    presetSelector.setBounds(8, 8, 123, 25);
    presetSelector.onChange = [this]() {
        int newState = presetSelector.getSelectedId();
        updatePanelRenderProfile(newState, selectedState);
        selectedState = newState;
        openGLComponent.setSelectedState(selectedState);
        };
    addAndMakeVisible(&presetSelector);
    for (int i = 0; i < openGL.getNumRenderStates(); i++) {
        addRenderPofile(openGL.getProfileComponent(i)); // Here they will be added to the presetSelector.
    }
    presetSelector.setSelectedId(DEFAULT_RENDER_STATE);

    openInApp.setButtonText("Open In App");
    openInApp.setBounds(8, 40, 123, 25);
    openInApp.onClick = [this] {
        // open qr code document window
        };
    addAndMakeVisible(openInApp);

    fullscreen.setButtonText("Fullscreen");
    fullscreen.setBounds(8, 72, 123, 25);
    fullscreen.onClick = [this] {
        openGLComponent.setFullScreen(true);
        };
    addAndMakeVisible(fullscreen);

    open.setButtonText("Open");
    open.setBounds(6, 104, 40, 25);
    open.onClick = [this] {
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        openChooser.launchAsync(flags, [this](const juce::FileChooser& chooser) {
            juce::File file = chooser.getResult();
            DBG("File selected for playback!");
            pluginProcessor.setNewTransportSource(file);
            });
        };
    addAndMakeVisible(open);

    play.setButtonText("Play");
    play.setBounds(50, 104, 40, 25);
    play.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
    play.onClick = [this] {
        processPlay();
        };
    addAndMakeVisible(play);

    stop.setButtonText("Stop");
    stop.setBounds(94, 104, 40, 25);
    stop.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::palevioletred);
    stop.onClick = [this] {
        processStop();
        };
    addAndMakeVisible(stop);

    settings.setButtonText("Settings");
    settings.setBounds(8, 136, 123, 25);
    settings.onClick = [this] {
        DBG("Launching the login panel!");
        settingsComponent.addToDesktop();
        settingsComponent.setVisible(true);
        settingsComponent.toFront(true);
        };
    addAndMakeVisible(settings);
}

SelectorTabPanel::~SelectorTabPanel(){
}

void SelectorTabPanel::paint (juce::Graphics& g) {
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::white);
    g.drawRect (getLocalBounds(), 1);
}

void SelectorTabPanel::resized() {
    auto area = getLocalBounds();
    for (auto* profile : renderProfiles)
        profile->setBounds(juce::Rectangle(0, 168, 140, 298));
}

void SelectorTabPanel::processPlay() {
    DBG("Audio Transport State is being changed to Playing by the play button in selector tab panel.");
    pluginProcessor.transportStateChanged(AudioVisualiserAudioProcessor::TransportState::Starting);
}

void SelectorTabPanel::processStop() {
    DBG("Audio Transport State is being changed to Stopping by the stop button in selector tab panel.");
    pluginProcessor.transportStateChanged(AudioVisualiserAudioProcessor::TransportState::Stopping);
}

void SelectorTabPanel::processRenderStateIncrement() {
    int newState = 1 + (presetSelector.getSelectedId() + 1 % presetSelector.getNumItems());
    presetSelector.setSelectedId(newState);
    updatePanelRenderProfile(newState, selectedState);
    selectedState = newState;
    openGLComponent.setSelectedState(selectedState);
}

void SelectorTabPanel::processRenderStateDecrement() {
    int newState = presetSelector.getSelectedId() - 1;
    if (newState < 1)
        newState = presetSelector.getNumItems();
    presetSelector.setSelectedId(newState);
    updatePanelRenderProfile(newState, selectedState);
    selectedState = newState;
    openGLComponent.setSelectedState(selectedState);
}