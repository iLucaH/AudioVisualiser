/*
  ==============================================================================

    ScreenSpaceQuad.h
    Created: 12 Sep 2026 10:45:38am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"
#include "RenderTarget.h"

#include <functional>

class ScreenSpaceQuad : public RenderState2D {
public:
    ScreenSpaceQuad(int id, juce::OpenGLContext& context, juce::String& fragShader, RenderTarget* renderTarget) : rTarget(renderTarget), RenderState2D(id, context, juce::String(R"(
    #version 330 core
    layout(location = 0) in vec4 position;

    out vec2 uv;

    void main() {
        gl_Position = position;

        // Convert position from [-1, 1] to [0, 1]
        uv = position.xy * 0.5 + 0.5;
    }
)"), fragShader) {
        renderProfile.setPresetName("Post Processing Effect");
    }

    void render() override {
        openGLContext.extensions.glUseProgram(getShaderProgramID());
        juce::gl::glActiveTexture(juce::gl::GL_TEXTURE0);
        juce::gl::glBindTexture(juce::gl::GL_TEXTURE_2D, rTarget->renderTextureId);

        GLuint textureUniform = openGLContext.extensions.glGetUniformLocation(getShaderProgramID(), "u_screenTexture");
        openGLContext.extensions.glUniform1i(textureUniform, 0);
        RenderState2D::render();
    }

    // Used for settings uniforms
    void render(const std::function<void(juce::OpenGLExtensionFunctions&, GLuint)>& beforeRender) {
        openGLContext.extensions.glUseProgram(getShaderProgramID());
        juce::gl::glActiveTexture(juce::gl::GL_TEXTURE0);
        juce::gl::glBindTexture(juce::gl::GL_TEXTURE_2D, rTarget->renderTextureId);

        beforeRender(openGLContext.extensions, getShaderProgramID());

        GLuint textureUniform = openGLContext.extensions.glGetUniformLocation(getShaderProgramID(), "u_screenTexture");
        openGLContext.extensions.glUniform1i(textureUniform, 0);
        RenderState2D::render();
    }

    RenderTarget* getRenderTarget() {
        return rTarget;
    }

private:
    RenderTarget* rTarget;
};