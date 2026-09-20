/*
  ==============================================================================

    RenderTarget.h
    Created: 12 Sep 2026 10:45:10am
    Author:  lucas

  ==============================================================================
*/

#include "Texture.h"

#pragma once

class RenderTarget {
public:
    RenderTarget(int w, int h) : width(w), height(h) {
        createRenderTarget();
    }

    GLuint renderTextureId;
    GLuint framebufferReference;

private:

    void createRenderTarget() {
        renderTextureId = create_gl_texture_id(width, height);

        juce::gl::glGenFramebuffers(1, &framebufferReference);
        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, framebufferReference);
        juce::gl::glFramebufferTexture2D(juce::gl::GL_FRAMEBUFFER, juce::gl::GL_COLOR_ATTACHMENT0, juce::gl::GL_TEXTURE_2D, renderTextureId, 0);
        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0);

        if (juce::gl::glCheckFramebufferStatus(juce::gl::GL_FRAMEBUFFER) != juce::gl::GL_FRAMEBUFFER_COMPLETE) {
            DBG("FBO creation incomplete!");
        }
    }

    void resize(int w, int h) {
        width = w;
        height = h;

        createRenderTarget();
    }

    int width, height;
};