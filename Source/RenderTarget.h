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

    GLuint renderTextureId;
    GLuint framebufferReference;

private:

    unsigned int create_texture_id(int width, int height) {
        unsigned int texture;

        juce::gl::glGenTextures(1, &texture);
        juce::gl::glBindTexture(juce::gl::GL_TEXTURE_2D, texture);
        juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_S, juce::gl::GL_REPEAT);
        juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_T, juce::gl::GL_REPEAT);
        juce::gl::glTexParameteri(
            juce::gl::GL_TEXTURE_2D,
            juce::gl::GL_TEXTURE_MIN_FILTER,
            juce::gl::GL_LINEAR
        );
        juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_MAG_FILTER, juce::gl::GL_LINEAR);

        juce::gl::glTexImage2D(juce::gl::GL_TEXTURE_2D, 0, juce::gl::GL_RGBA, width, height, 0, juce::gl::GL_RGBA, juce::gl::GL_UNSIGNED_BYTE, 0);

        return texture;
    }

    void createRenderTarget() {
        renderTextureId = create_texture_id(width, height);

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