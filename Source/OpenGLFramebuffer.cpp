/*
  ==============================================================================

    OpenGLFramebuffer.cpp
    Created: 11 Sep 2026 11:40:25pm
    Author:  lucas

  ==============================================================================
*/

#include "OpenGLFramebuffer.h"

OpenGLFramebuffer::~OpenGLFramebuffer()
{
    // OpenGL resources must be destroyed while the OpenGL
    // context is current.
    jassert(framebufferID == 0);
    jassert(textureID == 0);
}

void OpenGLFramebuffer::create(juce::OpenGLContext& context,
    int newWidth,
    int newHeight)
{
    jassert(context.isActive());

    destroy(context);

    width = newWidth;
    height = newHeight;

    juce::gl::glGenFramebuffers(1, &framebufferID);
    juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, framebufferID);

    juce::gl::glGenTextures(1, &textureID);
    juce::gl::glBindTexture(juce::gl::GL_TEXTURE_2D, textureID);

    juce::gl::glTexImage2D(juce::gl::GL_TEXTURE_2D,
        0,
        juce::gl::GL_RGBA8,
        width,
        height,
        0,
        juce::gl::GL_RGBA,
        juce::gl::GL_UNSIGNED_BYTE,
        nullptr);

    juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_MIN_FILTER, juce::gl::GL_LINEAR);
    juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_MAG_FILTER, juce::gl::GL_LINEAR);

    juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_S, juce::gl::GL_CLAMP_TO_EDGE);
    juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_T, juce::gl::GL_CLAMP_TO_EDGE);

    juce::gl::glFramebufferTexture2D(juce::gl::GL_FRAMEBUFFER,
        juce::gl::GL_COLOR_ATTACHMENT0,
        juce::gl::GL_TEXTURE_2D,
        textureID,
        0);

    const GLenum drawBuffers[] = { juce::gl::GL_COLOR_ATTACHMENT0 };
    juce::gl::glDrawBuffers(1, drawBuffers);

    const auto status = juce::gl::glCheckFramebufferStatus(juce::gl::GL_FRAMEBUFFER);

    if (status != juce::gl::GL_FRAMEBUFFER_COMPLETE)
    {
        DBG("Framebuffer incomplete: " << juce::String((int)status));
        jassertfalse;
    }

    juce::gl::glBindTexture(juce::gl::GL_TEXTURE_2D, 0);
    juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::destroy(juce::OpenGLContext& context)
{
    jassert(context.isActive());

    if (textureID != 0)
    {
        juce::gl::glDeleteTextures(1, &textureID);
        textureID = 0;
    }

    if (framebufferID != 0)
    {
        juce::gl::glDeleteFramebuffers(1, &framebufferID);
        framebufferID = 0;
    }

    width = 0;
    height = 0;
}

void OpenGLFramebuffer::resize(juce::OpenGLContext& context,
    int newWidth,
    int newHeight)
{
    if (newWidth <= 0 || newHeight <= 0)
        return;

    if (newWidth == width && newHeight == height && isValid())
        return;

    create(context, newWidth, newHeight);
}

void OpenGLFramebuffer::bind(juce::OpenGLContext& context, int viewportWidth, int viewportHeight) {
    jassert(context.isActive());

    juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, framebufferID);

    juce::gl::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    juce::gl::glClear(juce::gl::GL_COLOR_BUFFER_BIT);

    juce::gl::glViewport(0, 0, viewportWidth, viewportHeight);
}

void OpenGLFramebuffer::unbind(juce::OpenGLContext& context)
{
    jassert(context.isActive());

    juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::clear(juce::OpenGLContext& context, float r, float g, float b, float a) {
    bind(context, width, height);

    juce::gl::glClearColor(r, g, b, a);
    juce::gl::glClear(juce::gl::GL_COLOR_BUFFER_BIT);
}