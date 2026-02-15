/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioVisualiserAudioProcessorEditor::AudioVisualiserAudioProcessorEditor (AudioVisualiserAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), openGLComponent(p), selectorPanel(openGLComponent), tvOverlayComponent(openGLComponent) {
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

    videoComponent.addToDesktop();

    juce::Rectangle<int> area(100, 100, 600, 300);

    juce::RectanglePlacement placement(juce::RectanglePlacement::xMid
        | juce::RectanglePlacement::yMid);

    videoComponent.setBounds(area);
    videoComponent.setResizable(true, true);
    videoComponent.setUsingNativeTitleBar(true);
    videoComponent.setVisible(true);
    videoComponent.toFront(true);
}

AudioVisualiserAudioProcessorEditor::~AudioVisualiserAudioProcessorEditor() {
}

//==============================================================================
void AudioVisualiserAudioProcessorEditor::paint (juce::Graphics& g) {
}

void AudioVisualiserAudioProcessorEditor::resized() {
}
