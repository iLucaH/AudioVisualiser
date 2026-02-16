/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "OpenGLComponent.h"
#include "SelectorTabPanel.h"
#include "TVImageOverlay.h"
#include "CreateVideoComponent.h"

//==============================================================================
/**
*/
class AudioVisualiserAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AudioVisualiserAudioProcessorEditor (AudioVisualiserAudioProcessor&);
    ~AudioVisualiserAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioVisualiserAudioProcessor& audioProcessor;
    OpenGLComponent openGLComponent;
    TVImageOverlay tvOverlayComponent;

    unsigned int width, height;

    juce::TabbedComponent tabs{ juce::TabbedButtonBar::TabsAtTop };
    juce::ImageComponent tvComponent;
    SelectorTabPanel selectorPanel;
    CreateVideoComponent videoComponent;

    bool recorderSessionInitialised = false;
    juce::TextButton launchRecorder;

    int tick;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioVisualiserAudioProcessorEditor)
};
