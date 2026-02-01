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
    
    //addAndMakeVisible(openGLComponent);
    //openGLComponent.setBounds(15, 45, 640, 360);

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

    videoEncoder = std::make_unique<VideoEncoder>("C:/Users/lucas/OneDrive/Desktop/test/test.mp4", "h264_nvenc", width, height);
    videoEncoder.release();

}

AudioVisualiserAudioProcessorEditor::~AudioVisualiserAudioProcessorEditor() {
}

//==============================================================================
void AudioVisualiserAudioProcessorEditor::paint (juce::Graphics& g) {
}

void AudioVisualiserAudioProcessorEditor::resized() {
    //tabs.setBounds(getLocalBounds());
    //auto margianLeft = getWidth() * 0.025;
    //auto margianRight = getHeight() * 0.03 + 30;
    //auto scaleW = getWidth() * 0.95;
    //auto scaleY = getWidth() * 0.55;
    //openGLComponent.setBoundsScaled(juce::Rectangle<int>(margianLeft, margianRight, scaleW, scaleY));
}
