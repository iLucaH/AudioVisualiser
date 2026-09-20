/*
  ==============================================================================

    OpenGLFramebuffer.h
    Created: 11 Sep 2026 11:40:25pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class OpenGLFramebuffer {
public:
    OpenGLFramebuffer() = default;

    ~OpenGLFramebuffer();

    OpenGLFramebuffer(const OpenGLFramebuffer&) = delete;
    OpenGLFramebuffer& operator=(const OpenGLFramebuffer&) = delete;

    void create(juce::OpenGLContext& context, int width, int height);
    void destroy(juce::OpenGLContext& context);

    void resize(juce::OpenGLContext& context, int width, int height);

    void bind(juce::OpenGLContext& context, int width, int height);
    void unbind(juce::OpenGLContext& context);

    void clear(juce::OpenGLContext& context,
        float r = 0.0f,
        float g = 0.0f,
        float b = 0.0f,
        float a = 1.0f);

    GLuint getTextureID() const noexcept { return textureID; }
    GLuint getFramebufferID() const noexcept { return framebufferID; }

    int getWidth() const noexcept { return width; }
    int getHeight() const noexcept { return height; }

    bool isValid() const noexcept {
        return framebufferID != 0 && textureID != 0;
    }

private:
    GLuint framebufferID = 0;
    GLuint textureID = 0;

    int width = 0;
    int height = 0;
};