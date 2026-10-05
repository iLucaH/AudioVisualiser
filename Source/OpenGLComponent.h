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
#include "RingBuffer.h"
#include "Settings.h"

#include "PostProcessor.h"

//==============================================================================
/*
*/

#define RING_BUFFER_READ_SIZE 256
#define FFT_BIN_SIZE 128

class OpenGLComponent : public juce::Component, public juce::OpenGLRenderer {

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

public:
    OpenGLComponent(AudioVisualiserAudioProcessor&, ApplicationSettings& appSettings);
    ~OpenGLComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent& event) override;

    void setSelectedState(unsigned int state) {
        selectedState.store(state);
    }

    void resetVideoRecorder(int width, int height);

    juce::OpenGLContext openGLContext;
    juce::Rectangle<int> cacheBounds;
    bool openGLViewportActive = true;

    std::atomic<juce::String*> pendingEncoderFileName{ nullptr };
    std::atomic<bool> pendingStop{ false };

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

    bool renderStateExistsByName(const juce::String& shaderName) {
        for (const auto& renderState : renderStates) {
            if (renderState->getRenderProfile()->getPresetName() == shaderName)
                return true;
        }

        return false;
    }

    VideoEncoder* getVideoEncoder() { 
        return videoEncoder.get(); 
    }

    void resizeComponent(juce::Rectangle<int> visualiserArea);

    void setFullScreen(bool state) {
        juce::String s = state == true ? "true" : "false";
        DBG("OpenGLComponent full screen mode set to " << s << ".");
        fullScreenMode.store(state);
        //if (getPeer()) {
        //    getPeer()->setFullScreen(state);
        //}
        if (state) {
            pushBounds();
        } else {
            popBounds();
        }
    }

    bool isFullScreen() {
        return fullScreenMode.load();
    }

    void addRenderState(std::unique_ptr<RenderState> state) {
        renderStates.push_back(std::move(state));
    }
    
    int getNextAvailableRenderStateID() {
        return renderStates.size() + 1;
    }

    PostProcessor* getPostProcessor() {
        return &postProcessor;
    }

    bool isOpenGLEnabled() const {
        return openGLEnabled;
    }

    void setOpenGLEnabled(bool enabled) {
        openGLEnabled = enabled;
    }

    ApplicationSettings& getApplicationSettings() {
        return appSettings;
    }

private:
    AudioVisualiserAudioProcessor& processor;
    ApplicationSettings& appSettings;

    bool openGLEnabled = true;

    RingBuffer<float>& ringBuffer;
    juce::AudioBuffer<GLfloat> readBuffer;
    GLfloat visualizationBufferTD[RING_BUFFER_READ_SIZE];
    std::array<float, FFT_BIN_SIZE> visualizationBufferFD;

    std::atomic<unsigned int> selectedState{ 1 };
    unsigned int time = 0;
    std::vector<std::unique_ptr<RenderState>> renderStates;

    std::unique_ptr<VideoEncoder> videoEncoder;
    std::atomic<unsigned int> videoEncoderWidth{ 2 }, videoEncoderHeight{ 2 }; // 2 is just the minimum encoding size. The value is changed when the OpenGL Context is initialised.

    GLuint fbo;
    uint8_t* pixelBuffer;

    std::atomic<bool> fullScreenMode = { false };

    void popBounds() {
        setBounds(cacheBounds);
    }

    void pushBounds() {
        cacheBounds = getBounds();
        const juce::Displays::Display* mainDisplay = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
        if (mainDisplay != nullptr) {
            setBounds(mainDisplay->totalArea);
            setTopLeftPosition(mainDisplay->topLeftPhysical);
        }
    }

    PostProcessor postProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponent)
};