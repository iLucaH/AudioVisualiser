/*
  ==============================================================================

    OpenGLComponent.h
    Created: 18 Nov 2025 10:53:16pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

#include "RenderState.h"
#include "RenderState2D.h"
#include "PluginProcessor.h"
#include "VideoEncoder.h"

//==============================================================================
/*
*/

#define RING_BUFFER_READ_SIZE 256

class OpenGLComponent : public juce::Component, public juce::OpenGLRenderer {

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

public:
    OpenGLComponent(AudioVisualiserAudioProcessor&);
    ~OpenGLComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setSelectedState(unsigned int state) {
        selectedState = state;
    }

    void start() {
        openGLContext.setContinuousRepainting(true);
        popBounds();
    }
    void stop() {
        openGLContext.setContinuousRepainting(false);
        pushBounds();
    }

    juce::OpenGLContext openGLContext;
    juce::Rectangle<int> cacheBounds;
    bool openGLViewportActive = true;

    void setBoundsScaled(juce::Rectangle<int> bounds) {
        if (openGLViewportActive) {
            setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
        } else {
            cacheBounds.setX(bounds.getX());
            cacheBounds.setY(bounds.getY());
            cacheBounds.setWidth(bounds.getWidth());
            cacheBounds.setHeight(bounds.getHeight());
        }
    }

    int getNumRenderStates() {
        return renderStates.size();
    }

    RenderProfileComponent* getProfileComponent(int id) {
        return renderStates[id].get()->getRenderProfile();
    }

private:
    AudioVisualiserAudioProcessor& processor;
    RingBuffer<float>& ringBuffer;
    juce::AudioBuffer<GLfloat> readBuffer;
    GLfloat visualizationBuffer[RING_BUFFER_READ_SIZE];

    unsigned int selectedState = 1;
    unsigned int time = 0;
    std::vector<std::unique_ptr<RenderState>> renderStates;

    uint8_t* pixelBuffer;

    void addRenderState(std::unique_ptr<RenderState> state) {
        renderStates.push_back(std::move(state));
    }

    void popBounds() {
        setBounds(cacheBounds);
    }

    void pushBounds() {
        cacheBounds = getBounds();
        setTopLeftPosition(-10000, -10000);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponent)
};