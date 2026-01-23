/*
  ==============================================================================

    RenderState2D.cpp
    Created: 19 Nov 2025 10:52:40am
    Author:  lucas

  ==============================================================================
*/

#include "RenderState2D.h"

RenderState2D::RenderState2D(int id, juce::OpenGLContext& context, juce::String vert, juce::String frag) : RenderState(id, context, vert, frag) {}

void RenderState2D::init() {
    openGLContext.extensions.glGenBuffers(1, &vbo);
    openGLContext.extensions.glGenBuffers(1, &ibo);

    vertexBuffer = {
        { { -1.0f,  1.0f } },   // top-left
        { {  1.0f,  1.0f } },  // top-right
        { {  1.0f, -1.0f } },   // bottom-right
        { { -1.0f, -1.0f } }    // bottom-left
    };
    indexBuffer = {
        0, 1, 2,
        0, 2, 3
    };

    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo);
    openGLContext.extensions.glBufferData(juce::gl::GL_ARRAY_BUFFER, sizeof(Vertex) * vertexBuffer.size(),
        vertexBuffer.data(), juce::gl::GL_STATIC_DRAW);

    openGLContext.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ibo);
    openGLContext.extensions.glBufferData(juce::gl::GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexBuffer.size(),
        indexBuffer.data(), juce::gl::GL_STATIC_DRAW);
}

void RenderState2D::render() {
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo);
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Enable the position attribute.
    openGLContext.extensions.glVertexAttribPointer(0, 2, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, sizeof(Vertex), nullptr);
    openGLContext.extensions.glEnableVertexAttribArray(0);

    juce::gl::glDrawElements(juce::gl::GL_TRIANGLES, indexBuffer.size(), juce::gl::GL_UNSIGNED_INT, nullptr);

    openGLContext.extensions.glDisableVertexAttribArray(0);
    openGLContext.extensions.glDisableVertexAttribArray(1);
}

void RenderState2D::shutdown() {}