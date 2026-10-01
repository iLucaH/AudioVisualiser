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
void PostProcessor::init(int w, int h) {
    for (const auto& effect : postProcessEffects) {
        effect->init(w, h);
    }
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
    if (postProcessEffects.empty()) {
        return nullptr;
    }

    return postProcessEffects.front().get();
}

void PostProcessor::renderAll(int viewportWidth, int viewportHeight) {
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