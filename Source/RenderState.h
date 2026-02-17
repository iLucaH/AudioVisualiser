/*
  ==============================================================================

    RenderState.h
    Created: 19 Nov 2025 10:28:13am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderProfileComponent.h"

class RenderState {
public:
    RenderState(int id, juce::OpenGLContext&, juce::String vertexShader, juce::String fragmentShader);
    virtual ~RenderState() = default;

    virtual void init() = 0;
    virtual void shutdown() = 0;
    virtual void render() = 0;

    void initAndCompileShaders();

    void initNewFragmentShader(juce::String& fragmentShader);

    GLuint getShaderProgramID();

    bool isInititalised() {
        return isInit;
    }

    void setInitialised() {
        isInit = true;
    }

    RenderProfileComponent* getRenderProfile() {
        return &renderProfile;
    }
protected:
    int renderStateID;

    juce::OpenGLContext& openGLContext;

    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    juce::String vertexShader;
    juce::String fragmentShader;

    RenderProfileComponent renderProfile;

    bool isInit = false;
};