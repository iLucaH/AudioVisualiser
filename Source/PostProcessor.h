/*
  ==============================================================================

    PostProcessor.h
    Created: 12 Sep 2026 10:36:48am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include "PostProcessEffect.h"

class OpenGLComponent;

class PostProcessor {
public:
    PostProcessor(OpenGLComponent& glComponent);

    void init(int w, int h);
    bool noPostProcessorsEnabled();

    PostProcessEffect* peek();

    void renderAll(int viewportWidth, int viewportHeight);

    void addPostProcessEffect(std::unique_ptr<PostProcessEffect> postProcessEffect) {
        postProcessEffects.push_back(std::move(postProcessEffect));

        // Make sure that the array of post processing effects are stored in render priority order
        std::sort(
            postProcessEffects.begin(),
            postProcessEffects.end(),
            [](const auto& a, const auto& b)
            {
                return a->getPriority() < b->getPriority();
            }
        );
    }

    bool isEnabledGlobal() {
        return enabled;
    }

    void setEnabled(bool newEnabled) {
        enabled = newEnabled;
    }

    std::vector<std::unique_ptr<PostProcessEffect>>& getPostProcessEffects() {
        return postProcessEffects;
    }

private:

    bool enabled = true;

    OpenGLComponent& openGLComponent;

    std::vector<std::unique_ptr<PostProcessEffect>> postProcessEffects;

};