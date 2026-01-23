/*
  ==============================================================================

    TVImageOverlay.h
    Created: 24 Nov 2025 10:32:36pm
    Author:  lucas

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "OpenGLComponent.h"

class TVImageOverlay : public juce::Component
{
public:
    TVImageOverlay(OpenGLComponent& c) {
        tvImage = juce::ImageCache::getFromMemory(BinaryData::tv_png, BinaryData::tv_pngSize);
    }

    void paint(juce::Graphics& g) override
    {
        if (!tvImage.isNull())
            g.drawImageWithin(tvImage, 0, 0, 1080, 544, juce::RectanglePlacement::fillDestination, false);
    }

private:
    juce::Image tvImage;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TVImageOverlay)
};
