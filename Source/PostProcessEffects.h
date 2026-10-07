/*
  ==============================================================================

    PostProcessEffects.h
    Created: 4 Oct 2026 9:38:30pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Uniform_Component_Type {
    inline constexpr auto FLOAT_INPUT = "floatinput";
    inline constexpr auto INT_INPUT = "intinput";
    inline constexpr auto FLOAT_SLIDER = "floatslider";
    inline constexpr auto INT_SLIDER = "intslider";
    inline constexpr auto RGB_PICKER = "rgbpicker";
    inline constexpr auto BOOLEAN_BUTTON = "boolean";
}

struct PostProcessingEffectStruct {
    int id;
    juce::String name;
    int priority;
    bool enabled;
    juce::String fragmentShader;
    juce::var uniforms;
};

inline juce::var createFloatInputComponent(juce::String name, juce::String uniformHandle, float value) {
    auto* property = new juce::DynamicObject();
    property->setProperty("name", name);
    property->setProperty("handle", uniformHandle);
    property->setProperty("type", Uniform_Component_Type::FLOAT_INPUT);
    property->setProperty("value", value);
    property->setProperty("default", value);
    return property;
}

inline juce::var createIntInputComponent(juce::String name, juce::String uniformHandle, int value) {
    auto* property = new juce::DynamicObject();
    property->setProperty("name", name);
    property->setProperty("handle", uniformHandle);
    property->setProperty("type", Uniform_Component_Type::INT_INPUT);
    property->setProperty("value", value);
    property->setProperty("default", value);
    return property;
}

inline juce::var createFloatSliderComponent(juce::String name, juce::String uniformHandle, float value, float min, float max) {
    auto* property = new juce::DynamicObject();
    property->setProperty("name", name);
    property->setProperty("handle", uniformHandle);
    property->setProperty("type", Uniform_Component_Type::FLOAT_SLIDER);
    property->setProperty("value", value);
    property->setProperty("default", value);
    property->setProperty("min", min);
    property->setProperty("max", max);
    return property;
}

inline juce::var createIntSliderComponent(juce::String name, juce::String uniformHandle, int value, int min, int max) {
    auto* property = new juce::DynamicObject();
    property->setProperty("name", name);
    property->setProperty("handle", uniformHandle);
    property->setProperty("type", Uniform_Component_Type::INT_SLIDER);
    property->setProperty("value", value);
    property->setProperty("default", value);
    property->setProperty("min", min);
    property->setProperty("max", max);
    return property;
}

inline juce::var createRGBPickerComponent(juce::String name, juce::String uniformHandle, float red, float green, float blue) {
    auto makeRGB = [=]() {
        auto* rgb = new juce::DynamicObject();
        rgb->setProperty("red", red);
        rgb->setProperty("green", green);
        rgb->setProperty("blue", blue);
        return juce::var(rgb);
        };

    auto* property = new juce::DynamicObject();
    property->setProperty("name", name);
    property->setProperty("handle", uniformHandle);
    property->setProperty("type", Uniform_Component_Type::RGB_PICKER);
    property->setProperty("value", makeRGB());
    property->setProperty("default", makeRGB());   // separate object so value/default aren't shared
    return property;
}

inline juce::var createBooleanButtonComponent(juce::String name, juce::String uniformHandle, bool value) {
    auto* property = new juce::DynamicObject();
    property->setProperty("name", name);
    property->setProperty("handle", uniformHandle);
    property->setProperty("type", Uniform_Component_Type::BOOLEAN_BUTTON);
    property->setProperty("value", value);
    property->setProperty("default", value);
    return property;
}

// ==============================================================================
// Original effects (ids 0 to 4), now with uniforms
// ==============================================================================

inline PostProcessingEffectStruct createWaveDistortionPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Amplitude", "u_amplitude", 12.0f, 0.0f, 50.0f));          // x 0.001 in shader
    uniforms.add(createFloatSliderComponent("Row Waves", "u_rowFrequency", 30.0f, 0.0f, 100.0f));
    uniforms.add(createFloatSliderComponent("Column Waves", "u_columnFrequency", 25.0f, 0.0f, 100.0f));
    uniforms.add(createFloatSliderComponent("Radial Waves", "u_radialFrequency", 13.5f, 0.0f, 60.0f));
    uniforms.add(createFloatSliderComponent("Phase", "u_phase", 0.0f, 0.0f, 6.28f));
    return {
        0, "Wave Distortion", 0, true, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_amplitude;
            uniform float u_rowFrequency;
            uniform float u_columnFrequency;
            uniform float u_radialFrequency;
            uniform float u_phase;
 
            void main()
            {
                vec2 centre = uv - 0.5;
                float dist = length(centre);
                float amp = u_amplitude * 0.001;
 
                vec2 distortedUV = uv;
 
                // x wobbles along y (rows), y wobbles along x (columns)
                distortedUV.x += sin(uv.y * u_rowFrequency + dist * u_radialFrequency + u_phase) * amp;
                distortedUV.y += cos(uv.x * u_columnFrequency + dist * u_radialFrequency * 0.8 + u_phase) * amp;
 
                fragColor = texture(u_screenTexture, distortedUV);
            })",
        juce::var(uniforms)
    };
}

inline PostProcessingEffectStruct createAberrationPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Strength", "u_strength", 6.0f, 0.0f, 50.0f));            // x 0.001 in shader
    uniforms.add(createFloatSliderComponent("Falloff", "u_falloff", 1.5f, 0.5f, 4.0f));
    uniforms.add(createFloatSliderComponent("Centre X", "u_centreX", 0.5f, 0.0f, 1.0f));
    uniforms.add(createFloatSliderComponent("Centre Y", "u_centreY", 0.5f, 0.0f, 1.0f));
    return {
        1, "Aberration", 1, true, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_strength;
            uniform float u_falloff;
            uniform float u_centreX;
            uniform float u_centreY;
 
            void main() {
                // Direction and distance from the chosen centre
                vec2 dir  = uv - vec2(u_centreX, u_centreY);
                float dist = length(dir);
 
                // Offset grows toward the edges
                vec2 offset = normalize(dir + 1e-6) * pow(dist, u_falloff) * (u_strength * 0.001) * 2.0;
 
                // Sample each channel at a slightly different position
                float r = texture(u_screenTexture, uv + offset).r;
                vec2  ga = texture(u_screenTexture, uv).ga;
                float b = texture(u_screenTexture, uv - offset).b;
 
                fragColor = vec4(r, ga.x, b, ga.y);
            })",
        juce::var(uniforms)
    };
}

inline PostProcessingEffectStruct createVignettePostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Intensity", "u_intensity", 1.0f, 0.0f, 1.0f));
    uniforms.add(createFloatSliderComponent("Inner Radius", "u_innerRadius", 0.35f, 0.0f, 1.0f));
    uniforms.add(createFloatSliderComponent("Outer Radius", "u_outerRadius", 1.0f, 0.0f, 1.5f));
    uniforms.add(createFloatSliderComponent("Roundness", "u_roundness", 1.0f, 0.0f, 1.0f));           // 0 = follows screen shape, 1 = circle
    uniforms.add(createRGBPickerComponent("Colour", "u_colour", 0.0f, 0.0f, 0.0f));
    return {
        2, "Vignette", 2, true, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_intensity;
            uniform float u_innerRadius;
            uniform float u_outerRadius;
            uniform float u_roundness;
            uniform vec3 u_colour;
 
            void main() {
                vec4 colour = texture(u_screenTexture, uv);
 
                // Aspect ratio comes from the texture itself, so no manual uniform is needed
                vec2 size = vec2(textureSize(u_screenTexture, 0));
                float aspect = size.x / size.y;
 
                vec2 centre = uv - 0.5;
                centre.x *= mix(1.0, aspect, u_roundness);
                float dist = length(centre);
 
                // smoothstep is undefined when edge0 >= edge1, so keep inner below outer
                float inner = min(u_innerRadius, u_outerRadius - 0.001);
                float vignette = 1.0 - smoothstep(inner, u_outerRadius, dist);
                vignette = mix(1.0, vignette, u_intensity);
 
                colour.rgb = mix(u_colour, colour.rgb, vignette);
                fragColor = colour;
            })",
        juce::var(uniforms)
    };
}

// Strongly order-dependent: blocky mosaic.
// Pixelate BEFORE wave dist  -> the blocks get warped into wobbly shapes.
// Pixelate AFTER wave dist   -> clean, perfectly square blocks.
// Pixelate BEFORE aberration -> colour fringes survive on block edges.
// Pixelate AFTER aberration  -> fringes are quantised away.
inline PostProcessingEffectStruct createPixelatePostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createIntSliderComponent("Block Size (px)", "u_blockSize", 20, 1, 128));
    uniforms.add(createFloatSliderComponent("Mix", "u_mix", 1.0f, 0.0f, 1.0f));
    return {
        3, "Pixelate", 3, true, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform int u_blockSize;
            uniform float u_mix;
 
            void main()
            {
                // Square blocks measured in real pixels
                vec2 size = vec2(textureSize(u_screenTexture, 0));
                vec2 grid = size / float(max(u_blockSize, 1));
 
                vec2 blockUV = (floor(uv * grid) + 0.5) / grid;
                vec4 pixelated = texture(u_screenTexture, blockUV);
                vec4 original = texture(u_screenTexture, uv);
 
                fragColor = mix(original, pixelated, u_mix);
            })",
        juce::var(uniforms)
    };
}

// Colour invert.
// Invert BEFORE vignette -> the screen edges fade to black.
// Invert AFTER vignette  -> the screen edges fade to white.
inline PostProcessingEffectStruct createInvertPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Amount", "u_amount", 1.0f, 0.0f, 1.0f));
    uniforms.add(createBooleanButtonComponent("Invert Red", "u_invertRed", true));
    uniforms.add(createBooleanButtonComponent("Invert Green", "u_invertGreen", true));
    uniforms.add(createBooleanButtonComponent("Invert Blue", "u_invertBlue", true));
    return {
        4, "Invert", 4, true, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_amount;
            uniform bool u_invertRed;
            uniform bool u_invertGreen;
            uniform bool u_invertBlue;
 
            void main()
            {
                vec4 colour = texture(u_screenTexture, uv);
 
                vec3 mask = vec3(u_invertRed ? 1.0 : 0.0,
                                 u_invertGreen ? 1.0 : 0.0,
                                 u_invertBlue ? 1.0 : 0.0);
 
                vec3 inverted = mix(colour.rgb, 1.0 - colour.rgb, mask);
                fragColor = vec4(mix(colour.rgb, inverted, u_amount), colour.a);
            })",
        juce::var(uniforms)
    };
}

// ==============================================================================
// New effects (ids 5 to 13). These default to disabled so the stack isn't
// overwhelming out of the box. Switch them on from the UI.
// ==============================================================================

// Brightness / contrast / saturation / hue / gamma in one pass.
inline PostProcessingEffectStruct createColourGradingPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Brightness", "u_brightness", 0.0f, -1.0f, 1.0f));
    uniforms.add(createFloatSliderComponent("Contrast", "u_contrast", 1.0f, 0.0f, 3.0f));
    uniforms.add(createFloatSliderComponent("Saturation", "u_saturation", 1.0f, 0.0f, 3.0f));
    uniforms.add(createFloatSliderComponent("Hue Shift (deg)", "u_hue", 0.0f, -180.0f, 180.0f));
    uniforms.add(createFloatSliderComponent("Gamma", "u_gamma", 1.0f, 0.2f, 3.0f));
    return {
        5, "Colour Grading", 5, false, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_brightness;
            uniform float u_contrast;
            uniform float u_saturation;
            uniform float u_hue;
            uniform float u_gamma;
 
            // Column-major: rows are the standard RGB -> YIQ coefficients
            const mat3 RGB_TO_YIQ = mat3(0.299, 0.596, 0.211,
                                         0.587, -0.274, -0.523,
                                         0.114, -0.322, 0.312);
            const mat3 YIQ_TO_RGB = mat3(1.0, 1.0, 1.0,
                                         0.956, -0.272, -1.106,
                                         0.621, -0.647, 1.703);
 
            void main()
            {
                vec4 colour = texture(u_screenTexture, uv);
                vec3 c = colour.rgb;
 
                c += u_brightness;
                c = (c - 0.5) * u_contrast + 0.5;
 
                float lum = dot(c, vec3(0.2126, 0.7152, 0.0722));
                c = mix(vec3(lum), c, u_saturation);
 
                // Hue rotation in YIQ space
                float a = radians(u_hue);
                vec3 yiq = RGB_TO_YIQ * c;
                float i = yiq.y * cos(a) - yiq.z * sin(a);
                float q = yiq.y * sin(a) + yiq.z * cos(a);
                c = YIQ_TO_RGB * vec3(yiq.x, i, q);
 
                c = pow(max(c, vec3(0.0)), vec3(1.0 / u_gamma));
 
                fragColor = vec4(c, colour.a);
            })",
        juce::var(uniforms)
    };
}

// Multiplies the image by a colour (warm / cool / sepia-style looks).
inline PostProcessingEffectStruct createColourTintPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createRGBPickerComponent("Tint", "u_tint", 1.0f, 0.8f, 0.5f));
    uniforms.add(createFloatSliderComponent("Strength", "u_strength", 0.5f, 0.0f, 1.0f));
    uniforms.add(createBooleanButtonComponent("Preserve Brightness", "u_preserveLuminance", true));
    return {
        6, "Colour Tint", 6, false, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform vec3 u_tint;
            uniform float u_strength;
            uniform bool u_preserveLuminance;
 
            void main()
            {
                vec4 colour = texture(u_screenTexture, uv);
                vec3 tinted = colour.rgb * u_tint;
 
                if (u_preserveLuminance) {
                    const vec3 W = vec3(0.2126, 0.7152, 0.0722);
                    tinted *= dot(colour.rgb, W) / max(dot(tinted, W), 1e-4);
                }
 
                fragColor = vec4(mix(colour.rgb, clamp(tinted, 0.0, 1.0), u_strength), colour.a);
            })",
        juce::var(uniforms)
    };
}

// Single-pass disc blur using a golden-angle spiral. Cost scales with Samples.
inline PostProcessingEffectStruct createBlurPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Radius (px)", "u_radius", 4.0f, 0.0f, 32.0f));
    uniforms.add(createIntSliderComponent("Samples", "u_samples", 24, 4, 96));
    return {
        7, "Blur", 7, false, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_radius;
            uniform int u_samples;
 
            const float GOLDEN_ANGLE = 2.39996323;
 
            void main()
            {
                vec2 texel = 1.0 / vec2(textureSize(u_screenTexture, 0));
                int n = max(u_samples, 1);
 
                vec4 sum = vec4(0.0);
                for (int i = 0; i < n; ++i) {
                    float r = sqrt((float(i) + 0.5) / float(n));
                    float theta = float(i) * GOLDEN_ANGLE;
                    vec2 offset = vec2(cos(theta), sin(theta)) * r * u_radius * texel;
                    sum += texture(u_screenTexture, uv + offset);
                }
 
                fragColor = sum / float(n);
            })",
        juce::var(uniforms)
    };
}

// Unsharp-mask style sharpening using the four direct neighbours.
inline PostProcessingEffectStruct createSharpenPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Amount", "u_amount", 1.0f, 0.0f, 5.0f));
    uniforms.add(createFloatSliderComponent("Radius (px)", "u_radius", 1.0f, 0.5f, 4.0f));
    return {
        8, "Sharpen", 8, false, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_amount;
            uniform float u_radius;
 
            void main()
            {
                vec2 t = u_radius / vec2(textureSize(u_screenTexture, 0));
 
                vec4 centre = texture(u_screenTexture, uv);
                vec3 blur = (texture(u_screenTexture, uv + vec2(t.x, 0.0)).rgb
                           + texture(u_screenTexture, uv - vec2(t.x, 0.0)).rgb
                           + texture(u_screenTexture, uv + vec2(0.0, t.y)).rgb
                           + texture(u_screenTexture, uv - vec2(0.0, t.y)).rgb) * 0.25;
 
                vec3 sharpened = centre.rgb + (centre.rgb - blur) * u_amount;
                fragColor = vec4(clamp(sharpened, 0.0, 1.0), centre.a);
            })",
        juce::var(uniforms)
    };
}

// Static film grain. There's no time uniform in the system, so animate it by
// changing Seed (or add a u_time upload to PostProcessEffect later).
inline PostProcessingEffectStruct createFilmGrainPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Intensity", "u_intensity", 0.15f, 0.0f, 1.0f));
    uniforms.add(createFloatSliderComponent("Grain Size (px)", "u_grainSize", 1.0f, 1.0f, 8.0f));
    uniforms.add(createFloatSliderComponent("Midtone Bias", "u_response", 0.5f, 0.0f, 1.0f));
    uniforms.add(createBooleanButtonComponent("Monochrome", "u_monochrome", true));
    uniforms.add(createFloatInputComponent("Seed", "u_seed", 0.0f));
    return {
        9, "Film Grain", 9, false, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_intensity;
            uniform float u_grainSize;
            uniform float u_response;
            uniform bool u_monochrome;
            uniform float u_seed;
 
            // Hash without sine (Dave Hoskins)
            float hash(vec2 p)
            {
                vec3 p3 = fract(vec3(p.xyx) * 0.1031);
                p3 += dot(p3, p3.yzx + 33.33);
                return fract((p3.x + p3.y) * p3.z);
            }
 
            void main()
            {
                vec4 colour = texture(u_screenTexture, uv);
 
                vec2 cell = floor(gl_FragCoord.xy / max(u_grainSize, 1.0)) + vec2(u_seed * 17.0, u_seed * 31.0);
                vec3 noise = vec3(hash(cell), hash(cell + 17.1), hash(cell + 43.7)) - 0.5;
                if (u_monochrome) {
                    noise = vec3(noise.x);
                }
 
                // Grain is strongest in midtones, weaker in deep shadows / highlights
                float lum = dot(colour.rgb, vec3(0.2126, 0.7152, 0.0722));
                float response = mix(1.0, 4.0 * lum * (1.0 - lum), u_response);
 
                colour.rgb += noise * u_intensity * response * 2.0;
                fragColor = vec4(clamp(colour.rgb, 0.0, 1.0), colour.a);
            })",
        juce::var(uniforms)
    };
}

// CRT-style horizontal scanlines.
inline PostProcessingEffectStruct createScanlinesPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Intensity", "u_intensity", 0.3f, 0.0f, 1.0f));
    uniforms.add(createFloatSliderComponent("Line Spacing (px)", "u_spacing", 4.0f, 2.0f, 16.0f));
    uniforms.add(createFloatSliderComponent("Sharpness", "u_sharpness", 1.0f, 0.5f, 4.0f));
    uniforms.add(createFloatSliderComponent("Brightness Boost", "u_boost", 1.1f, 1.0f, 2.0f));
    return {
        10, "Scanlines", 10, false, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_intensity;
            uniform float u_spacing;
            uniform float u_sharpness;
            uniform float u_boost;
 
            void main()
            {
                vec4 colour = texture(u_screenTexture, uv);
 
                float s = 0.5 + 0.5 * sin(gl_FragCoord.y * 6.2831853 / u_spacing);
                s = pow(s, u_sharpness);
 
                float scan = 1.0 - u_intensity * (1.0 - s);
                colour.rgb *= scan * u_boost;
 
                fragColor = vec4(clamp(colour.rgb, 0.0, 1.0), colour.a);
            })",
        juce::var(uniforms)
    };
}

// Reduces each channel to a fixed number of levels.
inline PostProcessingEffectStruct createPosterizePostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createIntSliderComponent("Levels", "u_levels", 6, 2, 32));
    uniforms.add(createFloatSliderComponent("Mix", "u_mix", 1.0f, 0.0f, 1.0f));
    return {
        11, "Posterize", 11, false, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform int u_levels;
            uniform float u_mix;
 
            void main()
            {
                vec4 colour = texture(u_screenTexture, uv);
 
                float steps = float(max(u_levels, 2) - 1);
                vec3 posterized = floor(colour.rgb * steps + 0.5) / steps;
 
                fragColor = vec4(mix(colour.rgb, posterized, u_mix), colour.a);
            })",
        juce::var(uniforms)
    };
}

// Sobel edge detection on luminance. Either shows edges on black, or overlays
// them on the original image (toon-outline look).
inline PostProcessingEffectStruct createEdgeDetectPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Strength", "u_strength", 1.0f, 0.0f, 5.0f));
    uniforms.add(createFloatSliderComponent("Threshold", "u_threshold", 0.1f, 0.0f, 1.0f));
    uniforms.add(createFloatSliderComponent("Thickness (px)", "u_thickness", 1.0f, 0.5f, 3.0f));
    uniforms.add(createRGBPickerComponent("Edge Colour", "u_edgeColour", 1.0f, 1.0f, 1.0f));
    uniforms.add(createBooleanButtonComponent("Overlay On Image", "u_overlay", false));
    return {
        12, "Edge Detect", 12, false, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_strength;
            uniform float u_threshold;
            uniform float u_thickness;
            uniform vec3 u_edgeColour;
            uniform bool u_overlay;
 
            float luma(vec2 p)
            {
                return dot(texture(u_screenTexture, p).rgb, vec3(0.2126, 0.7152, 0.0722));
            }
 
            void main()
            {
                vec4 colour = texture(u_screenTexture, uv);
                vec2 t = u_thickness / vec2(textureSize(u_screenTexture, 0));
 
                float tl = luma(uv + vec2(-t.x,  t.y));
                float tc = luma(uv + vec2( 0.0,  t.y));
                float tr = luma(uv + vec2( t.x,  t.y));
                float ml = luma(uv + vec2(-t.x,  0.0));
                float mr = luma(uv + vec2( t.x,  0.0));
                float bl = luma(uv + vec2(-t.x, -t.y));
                float bc = luma(uv + vec2( 0.0, -t.y));
                float br = luma(uv + vec2( t.x, -t.y));
 
                float gx = -tl - 2.0 * ml - bl + tr + 2.0 * mr + br;
                float gy = -tl - 2.0 * tc - tr + bl + 2.0 * bc + br;
 
                float edge = length(vec2(gx, gy)) * u_strength;
                edge = smoothstep(u_threshold, u_threshold + 0.1, edge);
 
                vec3 base = u_overlay ? colour.rgb : vec3(0.0);
                fragColor = vec4(mix(base, u_edgeColour, edge), colour.a);
            })",
        juce::var(uniforms)
    };
}

// Lens / CRT barrel distortion. Negative strength gives pincushion.
inline PostProcessingEffectStruct createBarrelDistortionPostProcessingEffect() {
    juce::Array<juce::var> uniforms;
    uniforms.add(createFloatSliderComponent("Strength", "u_strength", 0.2f, -1.0f, 1.0f));
    uniforms.add(createFloatSliderComponent("Zoom", "u_zoom", 1.0f, 0.5f, 2.0f));
    uniforms.add(createRGBPickerComponent("Border Colour", "u_borderColour", 0.0f, 0.0f, 0.0f));
    return {
        13, "Barrel Distortion", 13, false, R"(#version 330 core
 
            in vec2 uv;
            out vec4 fragColor;
 
            uniform sampler2D u_screenTexture;
            uniform float u_strength;
            uniform float u_zoom;
            uniform vec3 u_borderColour;
 
            void main()
            {
                vec2 cc = (uv - 0.5) * 2.0;     // -1..1 from centre
                float r2 = dot(cc, cc);
 
                vec2 distortedUV = 0.5 + 0.5 * cc * (1.0 + u_strength * r2) / u_zoom;
 
                if (any(lessThan(distortedUV, vec2(0.0))) || any(greaterThan(distortedUV, vec2(1.0)))) {
                    fragColor = vec4(u_borderColour, 1.0);
                } else {
                    fragColor = texture(u_screenTexture, distortedUV);
                }
            })",
        juce::var(uniforms)
    };
}

// Animated screen warping with multiple selectable warp designs.
inline PostProcessingEffectStruct createTimeWarpPostProcessingEffect() {
    juce::Array<juce::var> uniforms;

    uniforms.add(createIntSliderComponent(
        "Design",
        "u_design",
        0,
        0,
        9
    ));

    uniforms.add(createFloatSliderComponent(
        "Strength",
        "u_strength",
        0.15f,
        0.0f,
        1.0f
    ));

    uniforms.add(createFloatSliderComponent(
        "Speed",
        "u_speed",
        1.0f,
        0.0f,
        5.0f
    ));

    uniforms.add(createFloatSliderComponent(
        "Frequency",
        "u_frequency",
        3.0f,
        0.1f,
        20.0f
    ));

    uniforms.add(createFloatSliderComponent(
        "Twist",
        "u_twist",
        1.0f,
        -5.0f,
        5.0f
    ));

    uniforms.add(createFloatSliderComponent(
        "Zoom",
        "u_zoom",
        1.0f,
        0.5f,
        2.0f
    ));

    uniforms.add(createFloatSliderComponent(
        "Centre X",
        "u_centreX",
        0.5f,
        0.0f,
        1.0f
    ));

    uniforms.add(createFloatSliderComponent(
        "Centre Y",
        "u_centreY",
        0.5f,
        0.0f,
        1.0f
    ));

    uniforms.add(createBooleanButtonComponent(
        "Mirror",
        "u_mirror",
        false
    ));

    return {
        14,
        "Time Warp",
        14,
        false,

        R"(#version 330 core

        in vec2 uv;
        out vec4 fragColor;

        uniform sampler2D u_screenTexture;

        uniform float u_time;

        uniform int   u_design;
        uniform float u_strength;
        uniform float u_speed;
        uniform float u_frequency;
        uniform float u_twist;
        uniform float u_zoom;

        uniform float u_centreX;
        uniform float u_centreY;

        uniform bool u_mirror;


        // ------------------------------------------------------------
        // Helpers
        // ------------------------------------------------------------

        vec2 centreUV()
        {
            return uv - vec2(u_centreX, u_centreY);
        }


        vec2 aspectCorrect(vec2 p)
        {
            vec2 size = vec2(textureSize(u_screenTexture, 0));
            float aspect = size.x / size.y;

            p.x *= aspect;

            return p;
        }


        vec2 restoreAspect(vec2 p)
        {
            vec2 size = vec2(textureSize(u_screenTexture, 0));
            float aspect = size.x / size.y;

            p.x /= aspect;

            return p;
        }


        vec2 rotate(vec2 p, float angle)
        {
            float c = cos(angle);
            float s = sin(angle);

            return mat2(c, -s, s, c) * p;
        }


        // ------------------------------------------------------------
        // Warp 0 - Ripple
        // ------------------------------------------------------------

        vec2 rippleWarp(vec2 p, float time)
        {
            float dist = length(p);

            float wave =
                sin(dist * u_frequency - time * u_speed * 3.0)
                * u_strength;

            return p + normalize(p + vec2(0.00001)) * wave * dist;
        }


        // ------------------------------------------------------------
        // Warp 1 - Vortex
        // ------------------------------------------------------------

        vec2 vortexWarp(vec2 p, float time)
        {
            float dist = length(p);

            float falloff =
                1.0 - smoothstep(0.0, 1.2, dist);

            float angle =
                u_twist
                * falloff
                * sin(time * u_speed);

            return rotate(p, angle);
        }


        // ------------------------------------------------------------
        // Warp 2 - Spiral
        // ------------------------------------------------------------

        vec2 spiralWarp(vec2 p, float time)
        {
            float dist = length(p);

            float angle =
                dist * u_frequency
                + time * u_speed
                + u_twist;

            angle *= u_strength * 4.0;

            return rotate(p, angle);
        }


        // ------------------------------------------------------------
        // Warp 3 - Horizontal wave
        // ------------------------------------------------------------

        vec2 horizontalWave(vec2 p, float time)
        {
            p.y +=
                sin(
                    p.x * u_frequency
                    + time * u_speed
                )
                * u_strength;

            return p;
        }


        // ------------------------------------------------------------
        // Warp 4 - Vertical wave
        // ------------------------------------------------------------

        vec2 verticalWave(vec2 p, float time)
        {
            p.x +=
                sin(
                    p.y * u_frequency
                    + time * u_speed
                )
                * u_strength;

            return p;
        }


        // ------------------------------------------------------------
        // Warp 5 - Liquid
        // ------------------------------------------------------------

        vec2 liquidWarp(vec2 p, float time)
        {
            float xWave =
                sin(
                    p.y * u_frequency
                    + time * u_speed
                );

            float yWave =
                cos(
                    p.x * u_frequency * 1.37
                    - time * u_speed * 0.8
                );

            p.x += xWave * u_strength;
            p.y += yWave * u_strength;

            return p;
        }


        // ------------------------------------------------------------
        // Warp 6 - Pinch / bulge
        // ------------------------------------------------------------

        vec2 pinchWarp(vec2 p, float time)
        {
            float dist = length(p);

            float pulse =
                1.0
                + sin(time * u_speed) * u_strength;

            float falloff =
                1.0 - smoothstep(0.0, 1.2, dist);

            float scale =
                mix(1.0, pulse, falloff);

            return p * scale;
        }


        // ------------------------------------------------------------
        // Warp 7 - Shockwave
        // ------------------------------------------------------------

        vec2 shockwaveWarp(vec2 p, float time)
        {
            float dist = length(p);

            float wavePosition =
                fract(time * u_speed * 0.15) * 1.5;

            float wave =
                1.0
                - smoothstep(
                    0.0,
                    0.15,
                    abs(dist - wavePosition)
                );

            float direction =
                sin(time * u_speed * 0.5);

            p +=
                normalize(p + vec2(0.00001))
                * wave
                * u_strength
                * direction;

            return p;
        }


        // ------------------------------------------------------------
        // Warp 8 - Elastic
        // ------------------------------------------------------------

        vec2 elasticWarp(vec2 p, float time)
        {
            float dist = length(p);

            float wave =
                sin(
                    dist * u_frequency
                    - time * u_speed
                );

            float amount =
                wave
                * u_strength
                * (1.0 - smoothstep(0.0, 1.3, dist));

            return p * (1.0 + amount);
        }


        // ------------------------------------------------------------
        // Warp 9 - Cosmic / multi-wave
        // ------------------------------------------------------------

        vec2 cosmicWarp(vec2 p, float time)
        {
            float t = time * u_speed;

            float a =
                sin(
                    p.x * u_frequency
                    + t
                );

            float b =
                cos(
                    p.y * u_frequency * 1.3
                    - t * 1.2
                );

            float c =
                sin(
                    length(p) * u_frequency * 2.0
                    - t * 2.0
                );

            p.x += (a + c) * u_strength * 0.5;
            p.y += (b + c) * u_strength * 0.5;

            p = rotate(
                p,
                c * u_twist * u_strength
            );

            return p;
        }


        // ------------------------------------------------------------
        // Main
        // ------------------------------------------------------------

        void main()
        {
            float time = u_time;

            vec2 p = centreUV();

            // Correct the warp so circles remain circular
            p = aspectCorrect(p);

            // Zoom
            p /= max(u_zoom, 0.001);


            // Select warp design
            if (u_design == 0)
            {
                p = rippleWarp(p, time);
            }
            else if (u_design == 1)
            {
                p = vortexWarp(p, time);
            }
            else if (u_design == 2)
            {
                p = spiralWarp(p, time);
            }
            else if (u_design == 3)
            {
                p = horizontalWave(p, time);
            }
            else if (u_design == 4)
            {
                p = verticalWave(p, time);
            }
            else if (u_design == 5)
            {
                p = liquidWarp(p, time);
            }
            else if (u_design == 6)
            {
                p = pinchWarp(p, time);
            }
            else if (u_design == 7)
            {
                p = shockwaveWarp(p, time);
            }
            else if (u_design == 8)
            {
                p = elasticWarp(p, time);
            }
            else if (u_design == 9)
            {
                p = cosmicWarp(p, time);
            }


            // Restore texture aspect ratio
            p = restoreAspect(p);

            vec2 warpedUV =
                vec2(u_centreX, u_centreY)
                + p;


            // Optional mirror effect
            if (u_mirror)
            {
                warpedUV = abs(fract(warpedUV) * 2.0 - 1.0);
            }


            // Keep sampling inside the texture.
            warpedUV = clamp(
                warpedUV,
                vec2(0.001),
                vec2(0.999)
            );


            fragColor =
                texture(
                    u_screenTexture,
                    warpedUV
                );
        })",

        juce::var(uniforms)
    };
}