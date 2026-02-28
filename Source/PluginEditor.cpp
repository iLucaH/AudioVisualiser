/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioVisualiserAudioProcessorEditor::AudioVisualiserAudioProcessorEditor (AudioVisualiserAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), openGLComponent(p), selectorPanel(p, openGLComponent), tvOverlayComponent(openGLComponent), launchRecorder("Export"), login("Login"), videoComponent(openGLComponent) {
    width = 1080;
    height = 544;
    setSize (width, height);

    setResizable(true, true);
    const float ratio = 1.98529411f;
    getConstrainer()->setFixedAspectRatio(ratio);
    setResizeLimits(300, 250, 10000, 10000);
    centreWithSize(getWidth(), getHeight());
    
    addAndMakeVisible(openGLComponent);
    openGLComponent.setBounds(52, 42, 815, 460);

    addAndMakeVisible(tvOverlayComponent);
    tvOverlayComponent.setBounds(0, 0, 1080, 544);
    
    selectorPanel.setBounds(903, 28, 140, 498);
    addAndMakeVisible(selectorPanel);

    launchRecorder.setBounds(907, 496, 70, 28);
    launchRecorder.onClick = [this] {
        if (!recorderSessionInitialised) {
            DBG("Launching the recorder panel!");
            videoComponent.addToDesktop();

            videoComponent.setResizable(false, false);
            videoComponent.setUsingNativeTitleBar(true);
        }
        juce::Rectangle<int> area(100, 100, 600, 300);
        videoComponent.setBounds(area);
        videoComponent.setVisible(true);
        videoComponent.toFront(true);
    };
    addAndMakeVisible(launchRecorder);

    login.setBounds(981, 496, 58, 28);
    login.onClick = [this] {
        // Access HTTP Server
        // Get QR Code for Server Port from HTTP Server
        // Display QR Code In Login Session Component
        if (!loginSessionInitialised) {
            DBG("Launching the login panel!");
            loginComponent.addToDesktop();

            loginComponent.setResizable(false, false);
            loginComponent.setUsingNativeTitleBar(true);
        }
        juce::Rectangle<int> area(100, 100, 600, 600);
        loginComponent.setBounds(area);
        loginComponent.setVisible(true);
        loginComponent.toFront(true);
        };
    addAndMakeVisible(login);
}

AudioVisualiserAudioProcessorEditor::~AudioVisualiserAudioProcessorEditor() {
}

//==============================================================================
void AudioVisualiserAudioProcessorEditor::paint (juce::Graphics& g) {
}

void AudioVisualiserAudioProcessorEditor::resized() {
}
