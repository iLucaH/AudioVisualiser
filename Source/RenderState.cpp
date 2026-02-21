/*
  ==============================================================================

    RenderState.cpp
    Created: 19 Nov 2025 10:28:13am
    Author:  lucas

  ==============================================================================
*/

#include "RenderState.h"

RenderState::RenderState(int id, juce::OpenGLContext& context, juce::String vert, juce::String frag)
    : renderStateID(id), openGLContext(context), fragmentShader(std::make_shared<juce::String>(frag)), vertexShader(vert), renderProfile(id) {
    DBG(vertexShader);
}

void RenderState::initAndCompileShaders() {
    shaderProgram.reset(new juce::OpenGLShaderProgram(openGLContext));

    bool vertexOK = shaderProgram->addVertexShader(vertexShader);
    auto shaderPtr = std::atomic_load(&fragmentShader);
    bool fragmentOK = shaderProgram->addFragmentShader(*shaderPtr);
    bool linkOK = shaderProgram->link();

    if (!vertexOK) {
        std::cout << "Vertex shader failed:\n"
            << shaderProgram->getLastError() << std::endl;
        jassertfalse;
    }

    if (!fragmentOK) {
        std::cout << "Fragment shader failed:\n"
            << shaderProgram->getLastError() << std::endl;
        jassertfalse;
    }

    if (!linkOK) {
        std::cout << "Shader program failed to link:\n"
            << shaderProgram->getLastError() << std::endl;
        jassertfalse;
    }

    shaderProgram->use();

    init(); // initialize VBOs, IBOs, etc.

    setInitialised();
}

void RenderState::initNewFragmentShader(juce::String& shader) {
    DBG("Compiling a new fragment shader:");
    DBG(shader);
    std::atomic_store(&fragmentShader, std::make_shared<juce::String>(shader));
    initAndCompileShaders();
}

GLuint RenderState::getShaderProgramID() {
    if (shaderProgram)
        return shaderProgram->getProgramID();
    else
        return -1; // -1 for ERR
}