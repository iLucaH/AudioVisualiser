/*
  ==============================================================================

    RenderTarget.h
    Created: 12 Sep 2026 10:45:10am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class RenderTarget {
public:
    RenderTarget(int w, int h) : width(w), height(h) {
        createRenderTarget();
    }

    // To be called on the GL Thread.
    void resize(int w, int h) {
        if (w <= 0 || h <= 0 || (w == width && h == height))
            return;

        width = w;
        height = h;

        // Reallocate the storage of the existing texture.
        juce::gl::glBindTexture(juce::gl::GL_TEXTURE_2D, renderTextureId);
        allocateTextureStorage();
        juce::gl::glBindTexture(juce::gl::GL_TEXTURE_2D, 0);

        checkFramebuffer();
    }
    
    GLuint renderTextureId = 0;
    GLuint framebufferReference = 0;

private:
    void allocateTextureStorage() {
        juce::gl::glTexImage2D(juce::gl::GL_TEXTURE_2D, 0, juce::gl::GL_RGBA8, width, height, 0, juce::gl::GL_RGBA, juce::gl::GL_UNSIGNED_BYTE, nullptr);
    }

    void createRenderTarget() {
        juce::gl::glGenTextures(1, &renderTextureId);
        juce::gl::glBindTexture(juce::gl::GL_TEXTURE_2D, renderTextureId);
        juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_S, juce::gl::GL_CLAMP_TO_EDGE);
        juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_T, juce::gl::GL_CLAMP_TO_EDGE);
        juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_MIN_FILTER, juce::gl::GL_LINEAR);
        juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_MAG_FILTER, juce::gl::GL_LINEAR);
        allocateTextureStorage();
        juce::gl::glBindTexture(juce::gl::GL_TEXTURE_2D, 0);

        juce::gl::glGenFramebuffers(1, &framebufferReference);
        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, framebufferReference);
        juce::gl::glFramebufferTexture2D(juce::gl::GL_FRAMEBUFFER, juce::gl::GL_COLOR_ATTACHMENT0, juce::gl::GL_TEXTURE_2D, renderTextureId, 0);
        checkFramebuffer(); // must run while the FBO is still bound
        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0);
    }

    void checkFramebuffer() {
        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, framebufferReference);
        if (juce::gl::glCheckFramebufferStatus(juce::gl::GL_FRAMEBUFFER) != juce::gl::GL_FRAMEBUFFER_COMPLETE) {
            DBG("FBO incomplete!");
        }
        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0);
    }

    int width, height;
};