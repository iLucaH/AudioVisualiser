/*
  ==============================================================================

    Classic3_2D.h
    Created: 23 Nov 2025 10:09:05am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"

class Classic3_2D : public RenderState2D {
public:
    Classic3_2D(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
    #version 330 core
    layout(location = 0) in vec4 position;

    void main() {
        gl_Position = position;
    }
)"), juce::String(R"(
    #version 330 core

    uniform int time;
    uniform int mode;
    uniform float screenWidth;
    uniform float screenHeight;
    uniform float leftRMS;
    uniform float rightRMS;

    out vec4 outColour;

    void main() {
        vec2 midPoint = vec2(screenWidth / 2.0f, screenHeight / 2.0f);
        vec2 uv = gl_FragCoord.xy;

        float dist = distance(uv, midPoint);
        float maxDist = length(midPoint) * (0.5f + ((leftRMS + rightRMS)));
        float normDist = dist / maxDist;

        vec3 color;
        if (mode == 0)
            color = vec3(normDist, 1.0 - normDist, 0.5);
        else if (mode == 1)
            color = vec3(1.0 - normDist, (sin(time / 100.0f) + 1.0f) / 2.0f, normDist);
        else if (mode == 2)
            color = vec3((sin(time / 100.0f) + 1.0f) / 2.0f, normDist, 1.0 - normDist);
        outColour = vec4(color, 1.0);
    }
)")) {
        button.setBounds(0, 0, 25, 25);
        button.onClick = [this]() {
            setMode((mode + 1) % 3);
        };
        renderProfile.addComponent(&button);
        renderProfile.setPresetName("Classic3");
    }

void render() override {
        GLuint modeUniform = openGLContext.extensions.glGetUniformLocation(getShaderProgramID(), "mode");
        openGLContext.extensions.glUniform1i(modeUniform, mode.load());
        RenderState2D::render();
 }

void setMode(int m) {
    mode.store(m);
}

private:
    std::atomic<unsigned int> mode{ 0 };
    juce::ToggleButton button;
};