#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"
#include "Settings.h"
#include "EventBus.h"

// Loads an image and distorts it based on audio bands.
// Effects are bit flags, so they can be stacked
class ImageOverlay : public RenderState2D {
public:
    enum Effect : int {
        Pulse = 1 << 0,  // zoom on bass
        Shake = 1 << 1,  // jitter on bass
        Ripple = 1 << 2,  // radial waves on mids
        RGBSplit = 1 << 3,  // channel separation on bass + treble
        Glitch = 1 << 4,  // horizontal slice displacement on treble
        HueShift = 1 << 5,  // hue rotation on mids
        Mirror = 1 << 6,  // horizontal mirror
        Flash = 1 << 7,   // brightness kick on bass
        FlipX = 1 << 8,  // flip horizontally
        FlipY = 1 << 9   // flip vertically
    };

    ImageOverlay(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
    #version 330 core
    layout(location = 0) in vec4 position;

    void main() {
        gl_Position = position;
    }
)"), juce::String(R"(
    #version 330 core

    uniform int   time;
    uniform float leftRMS;
    uniform float rightRMS;
    uniform float screenWidth;
    uniform float screenHeight;
    uniform float audioBufferTD[256];
    uniform float audioBufferFD[128];

    uniform sampler2D uTexture;
    uniform vec2  uImageSize;
    uniform float uIntensity;
    uniform int   uEffects;
    uniform int   uHasImage;

    out vec4 outColour;

    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    vec3 hueRotate(vec3 c, float a) {
        const mat3 toYIQ = mat3(0.299, 0.596, 0.211,
                                0.587, -0.274, -0.523,
                                0.114, -0.322, 0.312);
        const mat3 toRGB = mat3(1.0, 1.0, 1.0,
                                0.956, -0.272, -1.106,
                                0.621, -0.647, 1.703);
        vec3 yiq = toYIQ * c;
        float cs = cos(a), sn = sin(a);
        yiq.yz = mat2(cs, -sn, sn, cs) * yiq.yz;
        return toRGB * yiq;
    }

    bool has(int bit) { return (uEffects & bit) != 0; }

    // Tune these to match how your FD buffer is scaled.
    const float TIME_SCALE = 0.001; // assumes `time` is milliseconds
    const float FD_GAIN    = 1.0;

    float bandAvg(int lo, int hi) {
        float sum = 0.0;
        for (int i = lo; i < hi; i++)
            sum += audioBufferFD[i];
        return clamp(sum / float(hi - lo) * FD_GAIN, 0.0, 1.0);
    }

    void main() {
        vec2 resolution = vec2(screenWidth, screenHeight);
        float uTime = float(time) * TIME_SCALE;
        vec2 uv = gl_FragCoord.xy / resolution;

        float bass   = bandAvg(0, 6);
        float mid    = bandAvg(6, 40);
        float treble = bandAvg(40, 128);
        float rms    = clamp((leftRMS + rightRMS) * 0.5, 0.0, 1.0);

        // "Cover" fit so the image fills the viewport without stretching.
        float ra = resolution.x / resolution.y;
        float ia = uImageSize.x / max(uImageSize.y, 1.0);
        vec2 s = ra > ia ? vec2(1.0, ia / ra) : vec2(ra / ia, 1.0);
        uv = (uv - 0.5) * s + 0.5;
        uv.y = 1.0 - uv.y; // images load top-down

        if (has(256)) uv.x = 1.0 - uv.x;
        if (has(512)) uv.y = 1.0 - uv.y;

        float I = uIntensity;

        if (has(64))  uv.x = 0.5 - abs(uv.x - 0.5);
        if (has(1))   uv = (uv - 0.5) / (1.0 + bass * 0.25 * I) + 0.5;
        if (has(2))   uv += vec2(sin(uTime * 60.0), cos(uTime * 47.0)) * bass * 0.02 * I;
        if (has(4)) {
            vec2 d = uv - 0.5;
            float r = length(d);
            uv += normalize(d + 1e-5) * sin(r * 40.0 - uTime * 6.0) * mid * 0.02 * I;
        }
        if (has(16)) {
            float row = floor(uv.y * 30.0);
            float n = hash(vec2(row, floor(uTime * 12.0)));
            if (n > 1.0 - treble * 0.5 * I)
                uv.x += (n - 0.5) * 0.2 * I;
        }

        vec3 col;
        if (uHasImage == 0) {
            // No image loaded yet: dark checkerboard placeholder.
            vec2 g = floor(gl_FragCoord.xy / 32.0);
            col = vec3(mod(g.x + g.y, 2.0) * 0.08 + 0.04);
        } else {
            vec2 cuv = clamp(uv, 0.0, 1.0);
            if (has(8)) {
                float off = (bass + treble) * 0.015 * I;
                col.r = texture(uTexture, clamp(cuv + vec2(off, 0.0), 0.0, 1.0)).r;
                col.g = texture(uTexture, cuv).g;
                col.b = texture(uTexture, clamp(cuv - vec2(off, 0.0), 0.0, 1.0)).b;
            } else {
                col = texture(uTexture, cuv).rgb;
            }
        }

        if (has(32))  col = hueRotate(col, uTime * 0.5 + mid * 3.0 * I);
        if (has(128)) col *= 1.0 + (bass * 0.5 + rms * 1.5) * 0.8 * I;

        outColour = vec4(col, 1.0);
    }
)")) {
        renderProfile.setPresetName("Image Overlay");
        renderProfile.setFrontEndPresets(createPresetSettings());

        renderProfile.setEventSubscription(
            [this](EventBus& eventBus) {
                eventBus.subscribe(Receive_Events::VisualiserImageLoad,
                    [this](const auto& args) {
                        handleLoadImage();
                        return juce::var(juce::String(""));
                    }
                );

                eventBus.subscribe(Receive_Events::VisualiserImageIntensityUp,
                    [this](const auto& args) {
                        float v = juce::jmin(3.0f, intensity.load() + 0.25f);
                        intensity.store(v);
                        return juce::var(juce::String("Intensity: ") + juce::String(v, 2));
                    }
                );

                eventBus.subscribe(Receive_Events::VisualiserImageIntensityDown,
                    [this](const auto& args) {
                        float v = juce::jmax(0.0f, intensity.load() - 0.25f);
                        intensity.store(v);
                        return juce::var(juce::String("Intensity: ") + juce::String(v, 2));
                    }
                );

                eventBus.subscribe(Receive_Events::VisualiserImageReset,
                    [this](const auto& args) {
                        effects.store(0);
                        intensity.store(1.0f);
                        return juce::var(juce::String("Effects cleared"));
                    }
                );

                // One handle per effect toggle.
                auto bindToggle = [this, &eventBus](const auto& eventName, int bit, const juce::String& label) {
                    eventBus.subscribe(eventName,
                        [this, bit, label](const auto& args) {
                            int now = effects.fetch_xor(bit) ^ bit;
                            return juce::var(label + ((now & bit) ? ": ON" : ": OFF"));
                        }
                    );
                };

                bindToggle(Receive_Events::VisualiserImagePulse, Pulse, "Pulse");
                bindToggle(Receive_Events::VisualiserImageShake, Shake, "Shake");
                bindToggle(Receive_Events::VisualiserImageRipple, Ripple, "Ripple");
                bindToggle(Receive_Events::VisualiserImageRGBSplit, RGBSplit, "RGB Split");
                bindToggle(Receive_Events::VisualiserImageGlitch, Glitch, "Glitch");
                bindToggle(Receive_Events::VisualiserImageHueShift, HueShift, "Hue Shift");
                bindToggle(Receive_Events::VisualiserImageMirror, Mirror, "Mirror");
                bindToggle(Receive_Events::VisualiserImageFlash, Flash, "Flash");
                bindToggle(Receive_Events::VisualiserImageFlipX, FlipX, "Flip X");
                bindToggle(Receive_Events::VisualiserImageFlipY, FlipY, "Flip Y");
            }
        );
    }

    // Runs on the GL thread.
    void render() override {
        // Upload a newly loaded image (texture work must happen on the GL thread).
        if (auto* img = pendingImage.exchange(nullptr)) {
            imageTexture.loadImage(*img);
            hasImage = imageTexture.getWidth() > 0;
            delete img;
        }
        juce::gl::glActiveTexture(juce::gl::GL_TEXTURE0);
        if (hasImage)
            imageTexture.bind();

        GLuint uTexture = openGLContext.extensions.glGetUniformLocation(getShaderProgramID(), "uTexture");
        openGLContext.extensions.glUniform1i(uTexture, 0);

        auto loc = [&](const char* name) { return openGLContext.extensions.glGetUniformLocation(getShaderProgramID(), name); };

        openGLContext.extensions.glUniform1i(loc("uTexture"), 0);
        openGLContext.extensions.glUniform2f(loc("uImageSize"), (float)imageTexture.getWidth(), (float)imageTexture.getHeight());
        openGLContext.extensions.glUniform1f(loc("uIntensity"), intensity.load());
        openGLContext.extensions.glUniform1i(loc("uEffects"), effects.load());
        openGLContext.extensions.glUniform1i(loc("uHasImage"), hasImage ? 1 : 0);

        RenderState2D::render();
    }

    void handleLoadImage() {
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        imageChooser.launchAsync(flags, [this](const juce::FileChooser& chooser) {
            juce::File file = chooser.getResult();
            if (!file.existsAsFile())
                return;
            juce::Image image = juce::ImageFileFormat::loadFrom(file);
            if (!image.isValid()) {
                DBG("Could not decode image: " << file.getFullPathName());
                return;
            }
            // Hand over to the GL thread. Freed in render().
            delete pendingImage.exchange(new juce::Image(image));
            });
    }

private:
    juce::var createPresetSettings() {
        auto root = new juce::DynamicObject();
        root->setProperty("value", renderProfile.getPresetName());
        root->setProperty("key", renderProfile.getRenderStateID());

        juce::Array<juce::var> content;

        auto makeButton = [](const juce::String& label, const auto& handler) {
            auto b = new juce::DynamicObject();
            b->setProperty("type", "button");
            b->setProperty("label", label);
            b->setProperty("clickhandler", handler);
            return juce::var(b);
            };

        auto makeRow = [](const juce::Array<juce::var>& items) {
            auto r = new juce::DynamicObject();
            r->setProperty("type", "row");
            r->setProperty("subcontent", items);
            return juce::var(r);
            };

        auto makeSpacer = []() {
            auto s = new juce::DynamicObject();
            s->setProperty("type", "spacer");
            s->setProperty("paddingTop", 15);
            s->setProperty("paddingBottom", 15);
            return juce::var(s);
            };

        // Load image
        content.add(makeButton("Load Image", Receive_Events::VisualiserImageLoad));
        content.add(makeSpacer());

        // Effect toggles, two per row.
        content.add(makeRow({ makeButton("Pulse",     Receive_Events::VisualiserImagePulse),
                              makeButton("Shake",     Receive_Events::VisualiserImageShake) }));
        content.add(makeRow({ makeButton("Ripple",    Receive_Events::VisualiserImageRipple),
                              makeButton("RGB Split", Receive_Events::VisualiserImageRGBSplit) }));
        content.add(makeRow({ makeButton("Glitch",    Receive_Events::VisualiserImageGlitch),
                              makeButton("Hue Shift", Receive_Events::VisualiserImageHueShift) }));
        content.add(makeRow({ makeButton("Mirror",    Receive_Events::VisualiserImageMirror),
                              makeButton("Flash",     Receive_Events::VisualiserImageFlash) }));
        content.add(makeSpacer());

        // Intensity + reset
        content.add(makeRow({ makeButton("Intensity -", Receive_Events::VisualiserImageIntensityDown),
                              makeButton("Intensity +", Receive_Events::VisualiserImageIntensityUp) }));
        content.add(makeRow({ makeButton("Flip X", Receive_Events::VisualiserImageFlipX),
                      makeButton("Flip Y", Receive_Events::VisualiserImageFlipY) }));
        content.add(makeButton("Clear Effects", Receive_Events::VisualiserImageReset));

        root->setProperty("subcontent", content);
        return juce::var(root);
    }

    juce::FileChooser imageChooser{ "Select Image",
                                     juce::File::getSpecialLocation(juce::File::userPicturesDirectory),
                                     "*.png;*.jpg;*.jpeg;*.bmp;*.gif" };

    juce::OpenGLTexture imageTexture;
    bool hasImage = false; // GL thread only

    std::atomic<juce::Image*> pendingImage{ nullptr };
    std::atomic<int>   effects{ Pulse | RGBSplit };
    std::atomic<float> intensity{ 1.0f };
};