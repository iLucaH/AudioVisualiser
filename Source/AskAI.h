/*
  ==============================================================================

    AskAI.h
    Created: 17 Feb 2026 11:32:16pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RenderState2D.h"
#include "AVAPIResolver.h"

class AskAI : public RenderState2D {
public:
    AskAI(int id, juce::OpenGLContext& context) : RenderState2D(id, context, juce::String(R"(
    #version 330 core
    layout(location = 0) in vec4 position;

    void main() {
        gl_Position = position;
    }
)"), juce::String(R"(
    #version 330 core

    out vec4 outColour;

    void main() {
        outColour = vec4(0.0f, 0.0f, 0.0f, 1.0);
    }
)")) {
        renderProfile.setPresetName("AskAI");

        // Submit prompt button logic here.
        submit.setButtonText("Click to submit prompt!");
        submit.setBounds(7, 237, 125, 25);
        submit.onClick = [this]() {
            // The API is not yet complete and therefore this code is in limbo.

            // Here a new thread should be dispatched that will call the rest API service for a GET request.
            // The returning result should be stored as pendingFragShader and pendingSubmit should be set to true.

            // e.g.
            // pendingSubmit.store(true);
            //auto* fragShader = new juce::String(FRAGMENT SHADER HERE);
            //pendingFragShader.store(fragShader);

            // Typing in the prompt text field happens in the same thread as this thread getting the text (Messanger thread).
            // Therefore there is no race condition here.
            const juce::String promptText = prompt.getText();
            juce::Thread::launch([this, promptText]() {
                    juce::String response = getPromptResponse(promptText);

                    // No response will be received if something goes wrong on the get request end.
                    if (response.length() > 0) {
                        auto* fragShader = new juce::String(response);
                        pendingFragShader.store(fragShader);
                        pendingSubmit.store(true);
                    }
                });
        };
        renderProfile.addComponent(&submit);

        // Type prompt logic here.
        prompt.setText("Type your prompt here!");
        prompt.setMultiLine(true, true);
        prompt.setReturnKeyStartsNewLine(true);
        prompt.setScrollbarsShown(true);
        prompt.setBounds(7, 7, 125, 225);
        renderProfile.addComponent(&prompt);
    }

    // This method will be called on the OpenGL Thread.
    void render() override {
        // Handle new shader compilation here on the GL Thread if the submit button has been pressed.
        if (pendingSubmit.exchange(false)) {
            DBG("New submit AI Fragment request is being processed");
            juce::String* shaderPtr = pendingFragShader.exchange(nullptr);
            if (shaderPtr) {
                DBG("New AI Fragment shader is being handled.");
                initNewFragmentShader(*shaderPtr);
                delete shaderPtr; // filePtr is created using new
            } else {
                DBG("New AI Fragment shader failed to init and compile!");
            }
        }
        RenderState2D::render();
    }


private:
    juce::TextButton submit;
    juce::TextEditor prompt;
    std::atomic<juce::String*> pendingFragShader{ nullptr };
    std::atomic<bool> pendingSubmit{ false };
};