/*
  ==============================================================================

    TimeDomain3_2D.h
    Created: 28 Nov 2025 2:29:54pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"

class TimeDomain3_2D : public RenderState2D {
public:
    TimeDomain3_2D(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
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

    void getAmplitudeForXPos (in float xPos, out float audioAmplitude) {
        float perfectSamplePosition = 255.0 * xPos / screenWidth;
        int leftSampleIndex = int (floor (perfectSamplePosition));
        int rightSampleIndex = int (ceil (perfectSamplePosition));
        audioAmplitude = mix (audioBufferTD[leftSampleIndex], audioBufferTD[rightSampleIndex], fract (perfectSamplePosition));
    }

    #define THICKNESS 0.02
    void main() {
        float y = gl_FragCoord.y / screenHeight;
        float amplitude = 0.0;
        getAmplitudeForXPos (gl_FragCoord.x, amplitude);
    
        amplitude = 0.5 - amplitude / 2.5;
        float r = abs (THICKNESS / (amplitude-y));

        gl_FragColor = vec4 (r - abs (r * 0.2), r - abs (r * 0.2), r - abs (r * 0.2), 1.0);
    }
        
)")) {
        renderProfile.setPresetName("TD3");
    }
};