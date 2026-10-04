/*
  ==============================================================================

    PostProcessEffects.h
    Created: 4 Oct 2026 9:38:30pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct PostProcessingEffectStruct {
    int id;
    juce::String name;
    int priority;
    bool enabled;
    juce::String fragmentShader;
    juce::var uniforms;
};

inline PostProcessingEffectStruct createWaveDistortionPostProcessingEffect() {
    return {
        0, "Wave Distortion", 0, true, R"(
            #version 330 core

            in vec2 uv;
            out vec4 fragColor;

            uniform sampler2D u_screenTexture;

            void main()
            {
                vec2 centre = uv - 0.5;
                float dist = length(centre);

                vec2 distortedUV = uv;

                distortedUV.x += sin(uv.y * 30.0 + dist * 15.0) * 0.012;
                distortedUV.y += cos(uv.x * 25.0 + dist * 12.0) * 0.012;

                fragColor = texture(u_screenTexture, distortedUV);
            })",
        juce::var()
    };
}

inline PostProcessingEffectStruct createAberrationPostProcessingEffect() {
    return {
        1, "Aberration", 1, true, R"(
            #version 330 core

            in vec2 uv;
            out vec4 fragColor;

            uniform sampler2D u_screenTexture;

            // Tweak these directly (or promote them to uniforms later)
            const float STRENGTH = 0.006;  // max channel offset at screen edges (in UV units)
            const float FALLOFF  = 1.5;    // >1 keeps the center sharper, pushes the effect outward

            void main() {
                // Direction and distance from screen center
                vec2 dir  = uv - vec2(0.5);
                float dist = length(dir);

                // Offset grows toward the edges
                vec2 offset = normalize(dir + 1e-6) * pow(dist, FALLOFF) * STRENGTH * 2.0;

                // Sample each channel at a slightly different position
                float r = texture(u_screenTexture, uv + offset).r;
                vec2  ga = texture(u_screenTexture, uv).ga;
                float b = texture(u_screenTexture, uv - offset).b;

                fragColor = vec4(r, ga.x, b, ga.y);
            })",
        juce::var()
    };
}

inline PostProcessingEffectStruct createVignettePostProcessingEffect() {
    return {
        2, "Vignette", 2, true, R"(
            #version 330 core

            in vec2 uv;
            out vec4 fragColor;

            uniform sampler2D u_screenTexture;

            void main()
            {
                vec4 colour = texture(u_screenTexture, uv);

                vec2 centre = uv - 0.5;
                float dist = length(centre);

                float vignette = 1.0 - smoothstep(0.25, 0.75, dist);

                colour.rgb *= vignette;

                fragColor = colour;
            })",
        juce::var()
    };
}

// Strongly order-dependent: blocky mosaic.
// Pixelate BEFORE wave dist  -> the blocks get warped into wobbly shapes.
// Pixelate AFTER wave dist   -> clean, perfectly square blocks.
// Pixelate BEFORE aberration -> colour fringes survive on block edges.
// Pixelate AFTER aberration  -> fringes are quantised away.
inline PostProcessingEffectStruct createPixelatePostProcessingEffect() {
    return {
        3, "Pixelate", 3, true, R"(
            #version 330 core

            in vec2 uv;
            out vec4 fragColor;

            uniform sampler2D u_screenTexture;

            const vec2 GRID = vec2(64.0, 36.0); // number of blocks across / down

            void main()
            {
                vec2 blockUV = (floor(uv * GRID) + 0.5) / GRID;
                fragColor = texture(u_screenTexture, blockUV);
            })",
        juce::var()
    };
}

// Colour invert.
// Invert BEFORE vignette -> the screen edges fade to black.
// Invert AFTER vignette  -> the screen edges fade to white.
inline PostProcessingEffectStruct createInvertPostProcessingEffect() {
    return {
        4, "Invert", 4, true, R"(
            #version 330 core

            in vec2 uv;
            out vec4 fragColor;

            uniform sampler2D u_screenTexture;

            void main()
            {
                vec4 colour = texture(u_screenTexture, uv);
                fragColor = vec4(1.0 - colour.rgb, colour.a);
            })",
        juce::var()
    };
}