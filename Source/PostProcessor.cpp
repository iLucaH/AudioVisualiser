/*
  ==============================================================================

    PostProcessor.cpp
    Created: 12 Sep 2026 10:36:48am
    Author:  lucas

  ==============================================================================
*/

#include "PostProcessor.h"
#include "OpenGLComponent.h"

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
    for (int i = 0; i < postProcessEffects.size(); ++i) {
        if (i == postProcessEffects.size() - 1) { // render to screen frame buffer
            juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0);
        }
        else { // render to the next post processing effect frame buffer
            juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, postProcessEffects[i + 1]->getScreenSpaceQuadFrameBuffer());
        }

        juce::gl::glClear(juce::gl::GL_COLOR_BUFFER_BIT);
        juce::gl::glViewport(0, 0, viewportWidth, viewportHeight);

        postProcessEffects[i]->render();
    }
}