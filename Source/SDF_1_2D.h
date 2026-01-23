/*
  ==============================================================================

    SDF_1_2D.h
    Created: 23 Nov 2025 6:13:10pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"

class SDF_1_2D : public RenderState2D {
public:
    SDF_1_2D(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
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
        vec2 midPoint = vec2(screenWidth / 2.0f, screenHeight / 2.0f);
        vec2 uv = gl_FragCoord.xy;

        float dist = distance(uv, midPoint);
        float maxDist = length(midPoint) * (0.5f + ((leftRMS + rightRMS)));
        float normDist = dist / maxDist;

        vec3 color = vec3((1.0 - normDist) * (leftRMS * 1000.0) * sin(uv.x * sin(float(time)) * 2.0), (sin(time / 100.0f) + 1.0f) / 2.0f, normDist * ((rightRMS * 1000.0) * cos(uv.y * sin(float(time)) * 2.0)));

        outColour = vec4(color, 1.0);
    }
)")) {
    }
};