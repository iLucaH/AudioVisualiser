/*
  ==============================================================================

    AVIOHandler.h
    Created: 20 Feb 2026 4:47:47pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This function is to be used with the AskAI.h render state to save generated shaders.
// The current implementation is to only generate the shader, however in the future the 
// option for the shader to create uniform state buttons may be used, in which the string
// would no longer be a shader literal, but a json string to be parsed.
inline bool saveRenderStateToFile(juce::String absolutePath, juce::String shader) {
    juce::File file = juce::File(absolutePath);
    return file.replaceWithText(shader);
}

inline juce::String getRenderStateFromFile(juce::String absolutePath) {
    juce::File file = juce::File(absolutePath);
    return file.loadFileAsString();
}