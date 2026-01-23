/*
  ==============================================================================

    TimeDomain2_2D.h
    Created: 28 Nov 2025 2:29:48pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"

class TimeDomain2_2D : public RenderState2D {
public:
    TimeDomain2_2D(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
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

    float getY(float x) {
        float perfectSamplePosition = 255.0 * x / screenWidth;
        int leftSampleIndex = int(floor(perfectSamplePosition));
        int rightSampleIndex = int(ceil(perfectSamplePosition));
        return mix(audioBufferTD[leftSampleIndex],
                     audioBufferTD[rightSampleIndex],
                     fract(perfectSamplePosition));
    }

	void main() {
	    // Normalize fragment coordinates to 0..1
	    vec2 uv = gl_FragCoord.xy / vec2(screenWidth, screenHeight); // uv coordinates
        vec2 audioBufferLocation = vec2(mix(0.0f, 255.0f, uv.x), mix(0.0f, 255.0f, uv.x));
	    
        float y = getY(audioBufferLocation.x) + 0.5f;
        vec2 p = vec2(uv.x, y);
        if (uv.y >= y + 0.005 || uv.y <= y - 0.005) {
            float dist = distance(uv, p);
            outColour = vec4(0.0f, 0.8f * dist, 1.0f * dist, 1.0f);
        } else {
            outColour = vec4(0.0f, 0.8f, 1.0f, 1.0f);
        }
    }
)")) {
        renderProfile.setPresetName("TD2");
    }
};