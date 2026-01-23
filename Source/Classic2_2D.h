/*
  ==============================================================================

    Classic2_2D.h
    Created: 23 Nov 2025 8:12:11am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"

class Classic2_2D : public RenderState2D {
public:
    Classic2_2D(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
    #version 330 core
    layout(location = 0) in vec4 position;

    void main() {
        gl_Position = position;
    }
)"), juce::String(R"(
    #version 330 core

    uniform int time;
    uniform float screenWidth;
    uniform float screenHeight;
    uniform float leftRMS;
    uniform float rightRMS;

    out vec4 outColour;

    void main() {
        vec2 midPoint = vec2(screenWidth / 2, screenHeight / 2);
        vec2 uv = gl_FragCoord.xy;

        float left = 0;
        if (leftRMS > rightRMS) {
            left = left + (leftRMS * 1000);
        } else {
            left = left - (rightRMS * 1000);   
        }
        float c = uv.x < midPoint.x - left ? 0.75 : 0.325;
        outColour = vec4(c, min((sin(time / 30.0f) + 1) / 2, 0.3), min((sin((time + 180.0f) / 60.0f) + 1) / 2, 0.7), 1);
    }
)")) {
    }
};