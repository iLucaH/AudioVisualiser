/*
  ==============================================================================

    PostProcessEffect.h
    Created: 26 Sep 2026 7:56:03pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"
#include "RenderTarget.h"
#include "PostProcessEffects.h"
#include "ScreenSpaceQuad.h"

class PostProcessEffect {
public:
    PostProcessEffect(const PostProcessingEffectStruct& def, juce::OpenGLContext& context)
        : id(def.id), priority(def.priority), glContext(context),
        fragmentShader(def.fragmentShader), name(def.name),
        enabled(def.enabled), uniforms(def.uniforms) {}

    void init(int w, int h) {
        renderTarget = std::make_unique<RenderTarget>(w, h);

        screenSpaceQuad = std::make_unique<ScreenSpaceQuad>(id, glContext, fragmentShader, renderTarget.get());
        screenSpaceQuad->initAndCompileShaders();
    }

    void render() {
        screenSpaceQuad->render([this](auto& gl, GLuint program) {
            useSetUniforms(gl, program);
        });
    }

    void ensureSize(int w, int h) {
        if (renderTarget)
            renderTarget->resize(w, h);
    }

    GLuint getScreenSpaceQuadFrameBuffer() {
        return screenSpaceQuad->getRenderTarget()->framebufferReference;
    }

    bool isEnabled() const {
        return enabled;
    }

    void setEnabled(const bool newEnabled) {
        enabled = newEnabled;
    }

    unsigned int getPriority() {
        return priority;
    }

    void setPriority(unsigned int newPriority) {
        priority = newPriority;
    }

    int getEffectID() const {
        return id;
    }

    juce::String getEffectName() const {
        return name;
    }

    juce::var getUniforms() const {
        const juce::ScopedLock sl(uniformLock);
        return uniforms;
    }

    void setUniforms(juce::var newUniforms) {
        const juce::ScopedLock sl(uniformLock);
        uniforms = std::move(newUniforms);
    }

private:
    int id;
    unsigned int priority;
    juce::OpenGLContext& glContext;
    juce::String fragmentShader, name;

    std::unique_ptr<RenderTarget> renderTarget;
    std::unique_ptr<ScreenSpaceQuad> screenSpaceQuad;

    bool enabled;

    mutable juce::CriticalSection uniformLock; // mutual exclusion for uniforms since its set in message thread but accessed in gl thread.
    juce::var uniforms;

    void useSetUniforms(juce::OpenGLExtensionFunctions& gl, GLuint program) {
        juce::var snapshot;
        {
            const juce::ScopedLock sl(uniformLock);
            snapshot = uniforms;
        }

        auto* array = snapshot.getArray();
        if (array == nullptr)
            return;

        for (const auto& uniform : *uniforms.getArray()) {
            auto* object = uniform.getDynamicObject();

            if (object == nullptr)
                continue;

            auto handle = object->getProperty("handle").toString();
            auto type = object->getProperty("type").toString();
            auto value = object->getProperty("value");

            GLint location = gl.glGetUniformLocation(program, handle.toRawUTF8());

            if (location == -1)
                continue;

            if (type == Uniform_Component_Type::FLOAT_INPUT || type == Uniform_Component_Type::FLOAT_SLIDER) {
                gl.glUniform1f(location, static_cast<float>(value));
            } else if (type == Uniform_Component_Type::INT_INPUT || type == Uniform_Component_Type::INT_SLIDER) {
                gl.glUniform1i(location, static_cast<int>(value));
            } else if (type == Uniform_Component_Type::BOOLEAN_BUTTON) {
                gl.glUniform1i(location, static_cast<bool>(value) ? 1 : 0);
            } else if (type == Uniform_Component_Type::RGB_PICKER) {
                auto* rgb = value.getDynamicObject();

                if (rgb == nullptr)
                    continue;

                gl.glUniform3f(
                    location,
                    static_cast<float>(rgb->getProperty("red")),
                    static_cast<float>(rgb->getProperty("green")),
                    static_cast<float>(rgb->getProperty("blue"))
                );
            }
        }
    }

};