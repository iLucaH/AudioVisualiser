/*
  ==============================================================================

    TimeDomain1_2D.h
    Created: 28 Nov 2025 2:29:41pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"

class TimeDomain1_2D : public RenderState2D {
public:
    TimeDomain1_2D(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
    #version 330 core
    layout(location = 0) in vec4 position;

    void main() {
        gl_Position = position;
    }
)"), juce::String(R"(
    #version 330 core

    uniform int time;
    uniform float leftRMS;
    uniform float rightRMS;
    uniform float screenWidth;
    uniform float screenHeight;
    uniform float audioBufferTD[256];

    out vec4 outColour;

    void main() {
        vec2 uv = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
        vec2 audioBufferLocation = vec2(mix(0.0f, 255.0f, uv.x), mix(0.0f, 255.0f, uv.x));
    
        outColour = vec4(
            0.0f,
            mix(0, 1, audioBufferTD[int(audioBufferLocation.x)]) * 1.5f,
            mix(0, 1, audioBufferTD[int(audioBufferLocation.y)]) * 1.5f,
            1.0);
    }
)")) {
        renderProfile.setPresetName("TD1");
    }
};