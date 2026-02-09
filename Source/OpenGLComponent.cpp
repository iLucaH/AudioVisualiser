/*
  ==============================================================================

    OpenGLComponent.cpp
    Created: 18 Nov 2025 10:53:16pm
    Author:  lucas

  ==============================================================================
*/

#include <JuceHeader.h>
#include "OpenGLComponent.h"
#include "RenderHeaders.h"

//==============================================================================
OpenGLComponent::OpenGLComponent(AudioVisualiserAudioProcessor &p) : processor(p), ringBuffer(p.getRingBuffer()), readBuffer(2, RING_BUFFER_READ_SIZE) {
    addRenderState(std::make_unique<Classic1_2D>(1, openGLContext));
    addRenderState(std::make_unique<Classic2_2D>(2, openGLContext));
    addRenderState(std::make_unique<Classic3_2D>(3, openGLContext));
    addRenderState(std::make_unique<Classic4_2D>(4, openGLContext));
    addRenderState(std::make_unique<TimeDomain1_2D>(5, openGLContext));
    addRenderState(std::make_unique<TimeDomain2_2D>(6, openGLContext));
    addRenderState(std::make_unique<TimeDomain3_2D>(7, openGLContext));
    addRenderState(std::make_unique<SDF_1_2D>(8, openGLContext));
    
    setOpaque(true); // Indicates that no part of this Component is transparent
    openGLContext.setRenderer(this); // Set this instance as the renderer for the context
    openGLContext.setContinuousRepainting(true); // Tell the context to repaint on a loop
    openGLContext.attachTo(*this); // Finally - we attach the context to this Component.
}

OpenGLComponent::~OpenGLComponent() {
    videoEncoder.release();
    openGLContext.detach();
}

void OpenGLComponent::paint(juce::Graphics& g) {
    // Everything that is drawn here overlays the OpenGL render.
}

void OpenGLComponent::resized() {
}

void OpenGLComponent::newOpenGLContextCreated() {
    juce::gl::glDebugMessageControl(juce::gl::GL_DONT_CARE, juce::gl::GL_DONT_CARE, juce::gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, juce::gl::GL_FALSE);
    for (int i = 0; i < renderStates.size(); i++) {
        RenderState* renderState = renderStates[i].get();
        if (renderState->isInititalised() == false) {
            renderState->initAndCompileShaders();
        }
    }
    videoEncoder = std::make_unique<VideoEncoder>(juce::String("C:/Users/lucas/OneDrive/Desktop/test/test.mp4"), getWidth(), getHeight());
    bool ret = videoEncoder->startRecordingSession();
    DBG("" + ret);
}

void OpenGLComponent::renderOpenGL() {
    time++;
    juce::OpenGLHelpers::clear(juce::Colours::black);
    if (selectedState < 1 || selectedState > renderStates.size())
        return;
    RenderState* renderState = renderStates[selectedState - 1].get();
    if (renderState->isInititalised() == false) {
        DBG("Shader not initialised!");
        return;
    }
    GLuint progID = renderState->getShaderProgramID();
    if (progID == -1) {
        DBG("Shader Program ID is invalid!");
        return;
    }
    openGLContext.extensions.glUseProgram(progID);

    GLuint timeUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "time");
    openGLContext.extensions.glUniform1i(timeUniform, time);

    GLuint leftRMSUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "leftRMS");
    openGLContext.extensions.glUniform1f(leftRMSUniform, processor.getRMS(0));
    GLuint rightRMSUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "rightRMS");
    openGLContext.extensions.glUniform1f(rightRMSUniform, processor.getRMS(1));

    GLuint screenWidthUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "screenWidth");
    GLuint screenHeightUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "screenHeight");
    auto scale = (float)openGLContext.getRenderingScale();
    openGLContext.extensions.glUniform1f(screenWidthUniform, getWidth() * scale);
    openGLContext.extensions.glUniform1f(screenHeightUniform, getHeight() * scale);

    ringBuffer.readSamples(readBuffer, RING_BUFFER_READ_SIZE);
    juce::FloatVectorOperations::clear(visualizationBuffer, RING_BUFFER_READ_SIZE);
    for (int i = 0; i < 2; ++i) { // Sum channels together
        juce::FloatVectorOperations::add(visualizationBuffer, readBuffer.getReadPointer(i, 0), RING_BUFFER_READ_SIZE);
    }
    GLuint visualizationUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "audioBufferTD");
    openGLContext.extensions.glUniform1fv(visualizationUniform, RING_BUFFER_READ_SIZE, visualizationBuffer);
    if (time == 1000) {
        videoEncoder->finishRecordingSession();
    } else if (time < 1000) {
        videoEncoder->addVideoFrame();
    }
    renderState->render();

}

void OpenGLComponent::openGLContextClosing() {
}