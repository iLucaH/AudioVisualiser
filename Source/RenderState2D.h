/*
  ==============================================================================

    RenderState2D.h
    Created: 19 Nov 2025 10:52:40am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState.h"

class RenderState2D : public RenderState {
public:
    RenderState2D(int id, juce::OpenGLContext& context, juce::String vert, juce::String frag);

    void init();
    void shutdown();
    void render();

protected:
    struct Vertex {
        float position[2];
        float colour[4];
    };

    std::vector<Vertex> vertexBuffer;
    std::vector<unsigned int> indexBuffer;

private:
    GLuint vbo; // Vertex buffer object.
    GLuint ibo; // Index buffer object.
};