/*
  ==============================================================================

    PostProcessor.cpp
    Created: 12 Sep 2026 10:36:48am
    Author:  lucas

  ==============================================================================
*/

#include "PostProcessor.h"
#include "OpenGLComponent.h"

#include <iterator>
#include <ranges>

PostProcessor::PostProcessor(OpenGLComponent& glComponent) : openGLComponent(glComponent) {
}

// to happen on OpenGL thread.
void PostProcessor::init(int w, int h, EventBus& eventBus) {
    for (const auto& effect : postProcessEffects) {
        effect->init(w, h);
    }

    juce::MessageManager::callAsync([this, &eventBus]() {
        // subscribe to events on the message thread.
        eventBus.subscribe(Receive_Events::VisualiserEffectGetAll,
            [this](const auto& args) {
                juce::Array<juce::var> effects;
                for (const auto& effect : postProcessEffects) {
                    auto* object = new juce::DynamicObject();
                    object->setProperty("name", effect->getEffectName());
                    object->setProperty("id", effect->getEffectID());
                    object->setProperty("priority", static_cast<int>(effect->getPriority()));
                    object->setProperty("enabled", effect->isEnabled());
                    object->setProperty("uniforms", effect->getUniforms());
                    effects.add(juce::var(object));
                }
                return juce::var(effects);
            }
        );
        eventBus.subscribe(Receive_Events::VisualiserEffectPostProcessorSwitch,
            [this](const auto& args) {
                setEnabled(!isEnabledGlobal());
                return juce::var(isEnabledGlobal());
            }
        );
        eventBus.subscribe(Receive_Events::VisualiserEffectPostProcessorSwitchEnabled,
            [this](const auto& args) {
                return juce::var(isEnabledGlobal());
            }
        );
        eventBus.subscribe(Receive_Events::VisualiserEffectUpdateEffect,
            [this](const auto& args) {
                if (args.size() < 6) {
                    return juce::var(false);
                }

                juce::String name = args[1];

                int id = args[2];
                PostProcessEffect* effect = nullptr;
                for (const auto& e : postProcessEffects) {
                    if (e->getEffectID() != id) {
                        continue;
                    }
                    effect = e.get();
                }
                if (effect == nullptr) {
                    return juce::var(false);
                }

                int priorityArg = static_cast<int>(args[3]);
                unsigned int priority = priorityArg >= 0 ? priorityArg : 0;

                bool enabled = args[4];

                juce::var uniforms = args[5];
                effect->setUniforms(uniforms);

                effect->setPriority(priority);
                effect->setEnabled(enabled);

                return juce::var(true);
            }
        );
    });
}

void PostProcessor::resizeTargets(juce::Rectangle<int> visualiserArea) {
}

void PostProcessor::updateRenderOrder(int screenWidth, int screenHeight) {
    std::vector<std::pair<unsigned int, PostProcessEffect*>> ordered;
    ordered.reserve(postProcessEffects.size());

    for (const auto& effect : postProcessEffects) {
        if (effect != nullptr && effect->isEnabled())
            ordered.emplace_back(effect->getPriority(), effect.get());
    }

    std::ranges::stable_sort(ordered, {}, &std::pair<unsigned int, PostProcessEffect*>::first);

    renderOrder.clear();
    for (const auto& entry : ordered) {
        entry.second->ensureSize(screenWidth, screenHeight);
        renderOrder.push_back(entry.second);
    }
}

PostProcessEffect* PostProcessor::peek() {
    return renderOrder.empty() ? nullptr : renderOrder.front();
}

bool PostProcessor::noPostProcessorsEnabled() {
    return renderOrder.empty();
}

void PostProcessor::renderAll(int viewportWidth, int viewportHeight) {
    for (size_t i = 0; i < renderOrder.size(); ++i) {
        PostProcessEffect* effect = renderOrder[i];
        const bool isLast = (i + 1 == renderOrder.size());

        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER,
            isLast ? 0 : renderOrder[i + 1]->getScreenSpaceQuadFrameBuffer());

        juce::gl::glClear(juce::gl::GL_COLOR_BUFFER_BIT);
        juce::gl::glViewport(0, 0, viewportWidth, viewportHeight);

        effect->render();
    }
}