/*
  ==============================================================================

    Classic1_2D.h
    Created: 23 Nov 2025 7:42:08am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"

class Classic1_2D : public RenderState2D {
public:
    Classic1_2D(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
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
    uniform float audioSampleData[256];

    out vec4 outColour;

    void main() {
        vec2 uv = gl_FragCoord.xy / 100.0;
        outColour = vec4(
            (leftRMS * 1000.0) * sin(uv.x * sin(float(time)) * 2.0),
            (rightRMS * 1000.0) * cos(uv.y * sin(float(time)) * 2.0),
            10.0,
            1.0
        );
    }
)")) {
        renderProfile.setPresetName("Classic1");
    }
};