/*
  ==============================================================================

    RenderState.cpp
    Created: 19 Nov 2025 10:28:13am
    Author:  lucas

  ==============================================================================
*/

#include "RenderState.h"

RenderState::RenderState(int id, juce::OpenGLContext& context, juce::String vert, juce::String frag)
    : renderStateID(id), openGLContext(context), fragmentShader(frag), vertexShader(vert), renderProfile(id) {
    DBG(vertexShader);
}

void RenderState::initAndCompileShaders() {
    shaderProgram.reset(new juce::OpenGLShaderProgram(openGLContext));

    bool vertexOK = shaderProgram->addVertexShader(vertexShader);
    bool fragmentOK = shaderProgram->addFragmentShader(fragmentShader);
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
    fragmentShader = shader;
    initAndCompileShaders();
}

GLuint RenderState::getShaderProgramID() {
    if (shaderProgram)
        return shaderProgram->getProgramID();
    else
        return -1; // -1 for ERR
}