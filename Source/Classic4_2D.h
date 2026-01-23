/*
  ==============================================================================

    Classic4_2D.h
    Created: 26 Nov 2025 11:57:37am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"

class Classic4_2D : public RenderState2D {
public:
    Classic4_2D(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
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
	    vec2 uv = gl_FragCoord.xy / vec2(screenWidth, screenHeight); // uv coordinates
        outColour = vec4(0.0f, uv.x * (leftRMS * rightRMS) * 1000.0f, uv.y * (leftRMS * rightRMS) * 1000.0f, 1.0f);
    }
)")) {
        renderProfile.setPresetName("Classic1");
    }
};