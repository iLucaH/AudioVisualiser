/*
  ==============================================================================

    PostProcessEffect.h
    Created: 26 Sep 2026 7:56:03pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"
#include "RenderTarget.h"
#include "ScreenSpaceQuad.h"

class PostProcessEffect {
public:
    PostProcessEffect(int id, const juce::String& name, int priority, juce::OpenGLContext& context, const juce::String& fragShader) : id(id), name(name), priority(priority), glContext(context), fragmentShader(fragShader) {
    }

    void init(int w, int h) {
        renderTarget = std::make_unique<RenderTarget>(w, h);

        screenSpaceQuad = std::make_unique<ScreenSpaceQuad>(
            id,
            glContext,
            fragmentShader,
            renderTarget.get()
        );
        screenSpaceQuad->initAndCompileShaders();
    }

    void render() {
        screenSpaceQuad->render();
    }

    GLuint getScreenSpaceQuadFrameBuffer() {
        return screenSpaceQuad->getRenderTarget()->framebufferReference;
    }

    bool isEnabled() {
        return enabled;
    }

    unsigned int getPriority() {
        return priority;
    }

    void setPriority(unsigned int newPriority) {
        priority = newPriority;
    }

private:
    int id;
    unsigned int priority;
    juce::OpenGLContext& glContext;
    juce::String fragmentShader, name;

    std::unique_ptr<RenderTarget> renderTarget;
    std::unique_ptr<ScreenSpaceQuad> screenSpaceQuad;

    bool enabled = true;
};