/*
  ==============================================================================

    Mesh.h
    Created: 29 Nov 2025 12:22:22pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

class Mesh {
public:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Mesh(juce::OpenGLContext& context, const std::vector<float>& vertices, const std::vector<unsigned int>& indices) : openGLContext(context), vertices(vertices), indices(indices) {
        setupMesh();
    }

    void Draw() {
        openGLContext.extensions.glBindVertexArray(VAO);
        juce::gl::glDrawElements(juce::gl::GL_TRIANGLES, static_cast<GLsizei>(indices.size()), juce::gl::GL_UNSIGNED_INT, 0);
        openGLContext.extensions.glBindVertexArray(0);
    }

private:
    juce::OpenGLContext& openGLContext;

    unsigned int VAO, VBO, EBO;

    void setupMesh() {
        openGLContext.extensions.glGenVertexArrays(1, &VAO);
        openGLContext.extensions.glGenBuffers(1, &VBO);
        openGLContext.extensions.glGenBuffers(1, &EBO);

        openGLContext.extensions.glBindVertexArray(VAO);
        openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, VBO);
        openGLContext.extensions.glBufferData(juce::gl::GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], juce::gl::GL_STATIC_DRAW);

        openGLContext.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, EBO);
        openGLContext.extensions.glBufferData(juce::gl::GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], juce::gl::GL_STATIC_DRAW);

        // Vertex positions
        openGLContext.extensions.glEnableVertexAttribArray(0);
        openGLContext.extensions.glVertexAttribPointer(0, 3, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 8 * sizeof(float), (void*)0);
        // Vertex normals
        openGLContext.extensions.glEnableVertexAttribArray(1);
        openGLContext.extensions.glVertexAttribPointer(1, 3, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        // Vertex texture coords
        openGLContext.extensions.glEnableVertexAttribArray(2);
        openGLContext.extensions.glVertexAttribPointer(2, 2, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        openGLContext.extensions.glBindVertexArray(0);
    }
};
