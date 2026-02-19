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

class AskAI : public RenderState2D, public juce::AsyncUpdater {
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
            // If we are already making a prompt submit request, then we should not continue with this one.
            if (pendingAPIRequest.load())
                return;

            const juce::String promptText = prompt.getText();
            // Launch the API request on a seperate thread because it is a blocking operation.
            juce::Thread::launch([this, promptText]() {
                pendingAPIRequest.store(true); 
                // Request the Fragment Shader from the API.
                juce::String response = getPromptResponse(promptText);
                // No response will be received if something goes wrong on the get request end.
                if (response.length() > 0) {
                    auto* fragShader = new juce::String(response); // The fragShader will be freed once exchanged in the render loop.
                    pendingFragShader.store(fragShader);
                    pendingSubmit.store(true);
                } else {
                    DBG("Could not resolve a prompt for the AskAI RenderState!");
                    displayStatusError.store(true);
                }
                pendingAPIRequest.store(false);
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

        statusText.setText("Test", juce::dontSendNotification);
        statusText.setBorderSize(juce::BorderSize<int>(2));
        statusText.setBounds(8, 265, 125, 25);
        renderProfile.addComponent(&statusText);
    }

    // Handle updating component entities on the messange thread. You can only update on the messange thread
    // and aquiring a MessageManagerLock on the render loop will block the GL thread until it aquires the lock.
    void handleAsyncUpdate() override {
        if (pendingAPIRequest.load()) {
            statusText.setColour(juce::Label::textColourId, juce::Colours::green);
            statusText.setText("Loading new shader...", juce::dontSendNotification);
        } else if (displayStatusError.load()) {
            statusText.setColour(juce::Label::textColourId, juce::Colours::red);
            statusText.setText("There was an error\nloading the shader!", juce::dontSendNotification);
        } else {
            // Display nothing if there is no updates or errors.
            statusText.setText("", juce::dontSendNotification);
        }
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
                displayStatusError.store(false);
            } else {
                DBG("New AI Fragment shader failed to init and compile!");
                displayStatusError.store(true);
            }
        }

        // Handle GUI updates as the state of the statusText is always changing.
        // This render loop is a good opportunity to make updates per frame.
        triggerAsyncUpdate();

        RenderState2D::render();
    }


private:
    juce::Label statusText;
    juce::TextButton submit;
    juce::TextEditor prompt;

    std::atomic<juce::String*> pendingFragShader{ nullptr };
    std::atomic<bool> pendingAPIRequest{ false };
    std::atomic<bool> pendingSubmit{ false };
    std::atomic<bool> displayStatusError{ false };
};