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
                if (args.size() < 5) {
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
                    DBG("Unable to update effect: " << name << ". Effect not found!");
                    return juce::var(false);
                }
                int priorityArg = static_cast<int>(args[3]);
                unsigned int priority = priorityArg >= 0 ? priorityArg : 0;
                bool enabled = args[4];

                effect->setPriority(priority);
                effect->setEnabled(enabled);
                DBG("Updating effect: " << name << ". Priority is now: " << priority << " and enabled is now " << (enabled ? "enabled." : "disabled."));

                return juce::var(true);
            }
        );
    });
}

bool PostProcessor::noPostProcessorsEnabled() {
    bool noneEnabled = std::ranges::none_of(
        postProcessEffects,
        [](const auto& effect) {
            return effect->isEnabled();
        }
    );
    return noneEnabled;
}

PostProcessEffect* PostProcessor::peek() {
    for (const auto& effect : postProcessEffects) {
        if (effect != nullptr && effect->isEnabled()) {
            return effect.get();
        }
    }
    return nullptr;
}

void PostProcessor::renderAll(int viewportWidth, int viewportHeight) {
    std::ranges::stable_sort(postProcessEffects, {}, [](const auto& e) {
        return e->getPriority();
    });
    auto enabledEffects = postProcessEffects | std::views::filter([](const auto& effect) {
        return effect != nullptr && effect->isEnabled();
    });

    for (auto it = enabledEffects.begin(); it != enabledEffects.end(); ++it) {
        auto& effect = *it;

        auto next = std::next(it);
        bool isLastEffect = next == enabledEffects.end();

        if (isLastEffect) { // Render to screen
            juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0);
        } else { // Render to the next enabled effect's framebuffer
            juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, (*next)->getScreenSpaceQuadFrameBuffer());
        }

        juce::gl::glClear(juce::gl::GL_COLOR_BUFFER_BIT);
        juce::gl::glViewport(0, 0, viewportWidth, viewportHeight);

        effect->render();
    }
}