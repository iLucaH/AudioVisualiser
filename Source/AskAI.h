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
#include "AVIOHandler.h"
#include "Settings.h"

class AskAI : public RenderState2D, public juce::AsyncUpdater {
public:
    AskAI(int id, juce::OpenGLContext& context, ApplicationSettings& applSettings) : RenderState2D(id, context, juce::String(R"(
    #version 330 core
    layout(location = 0) in vec4 position;

    void main() {
        gl_Position = position;
    }
)"), juce::String(R"(
    #version 330 core

    out vec4 outColour;

    void main() {
        outColour = vec4(0.1f, 0.1f, 0.1f, 1.0) * gl_FragCoord;
    }
)")), appSettings(applSettings),
      saveChooser("Save Shader", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.avrs"),
      loadChooser("Load Shader", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.avrs") {
        renderProfile.setPresetName("AI Generator");

        statusText.setText("", juce::dontSendNotification);
        statusText.setBorderSize(juce::BorderSize<int>(2));
        statusText.setBounds(8, 184, 125, 125);
        renderProfile.addComponent(&statusText);

        submit.setButtonText("Click to submit prompt!");
        submit.setBounds(7, 199, 125, 25);
        submit.onClick = [this]() {
            // If we are already making a prompt submit request, then we should not continue with this one.
            if (pendingAPIRequest.load())
                return;
            if (!appSettings.isAuth())
                return;

            const juce::String promptText = prompt.getText();
            // Launch the API request on a seperate thread because it is a blocking operation.
            juce::Thread::launch([this, promptText]() {
                pendingAPIRequest.store(true); 
                juce::String response = postPromptResponse(appSettings.getAuthJWT(), promptText);
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

        loadFromFile.setButtonText("File");
        loadFromFile.setBounds(5, 270, 41, 20);
        loadFromFile.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgreen);
        loadFromFile.onClick = [this]() {
            auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
            loadChooser.launchAsync(flags, [this](const juce::FileChooser& chooser) {
                juce::String filePath = chooser.getResult().getFullPathName();
                if (filePath.isEmpty())
                    return;
                DBG("Loading render state from " << filePath);
                juce::String renderState = getRenderStateFromFile(filePath);
                auto* fragShader = new juce::String(renderState);
                pendingFragShader.store(fragShader);
                pendingSubmit.store(true);
                });
            };
        renderProfile.addComponent(&loadFromFile);
        loadFromFile.setVisible(false);

        loadFromBackend.setButtonText("Cloud");
        loadFromBackend.setBounds(50, 270, 41, 20);
        loadFromBackend.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgreen);
        loadFromBackend.onClick = [this]() {
            if (!appSettings.isAuth()) {
                saveToBackend.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightcoral);
                delayColourChangeToComponent(&saveToBackend, juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
                return;
            }
            backenedListComboBox.setVisible(true);
            statusText.setVisible(false);
            };
        renderProfile.addComponent(&loadFromBackend);
        loadFromBackend.setVisible(false);

        cancelLoad.setButtonText("Cancel");
        cancelLoad.setBounds(94, 270, 41, 20);
        cancelLoad.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightcoral);
        cancelLoad.onClick = [this]() {
            loadFromFile.setVisible(false);
            loadFromBackend.setVisible(false);
            cancelLoad.setVisible(false);
            backenedListComboBox.setVisible(false);
            save.setVisible(true);
            load.setVisible(true);
            statusText.setVisible(true);
            };
        renderProfile.addComponent(&cancelLoad);
        cancelLoad.setVisible(false);

        saveToFile.setButtonText("File");
        saveToFile.setBounds(5, 270, 41, 20);
        saveToFile.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
        saveToFile.onClick = [this]() {
            auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting;
            saveChooser.launchAsync(flags, [this](const juce::FileChooser& chooser) {
                juce::String filePath = chooser.getResult().getFullPathName();
                if (filePath.isEmpty())
                    return;
                DBG("Saving render state to: " << filePath);
                auto shaderPtr = std::atomic_load(&fragmentShader);
                if (shaderPtr)
                    saveRenderStateToFile(filePath, *shaderPtr);
                });
            };
        renderProfile.addComponent(&saveToFile);
        saveToFile.setVisible(false);

        saveToBackend.setButtonText("Cloud");
        saveToBackend.setBounds(50, 270, 41, 20);
        saveToBackend.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
        saveToBackend.onClick = [this]() {
            if (!appSettings.isAuth()) {
                saveToBackend.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightcoral);
                delayColourChangeToComponent(&saveToBackend, juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
                return;
            }
            statusText.setVisible(false); // Hide status text so we can make room for the text box.
            saveNameEditor.setVisible(true);
            };
        renderProfile.addComponent(&saveToBackend);
        saveToBackend.setVisible(false);

        cancelSave.setButtonText("Cancel");
        cancelSave.setBounds(94, 270, 41, 20);
        cancelSave.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightcoral);
        cancelSave.onClick = [this]() {
            saveToFile.setVisible(false);
            saveToBackend.setVisible(false);
            cancelSave.setVisible(false);
            saveNameEditor.setVisible(false);
            save.setVisible(true);
            load.setVisible(true);
            statusText.setVisible(true);
            };
        renderProfile.addComponent(&cancelSave);
        cancelSave.setVisible(false);

        save.setButtonText("Save");
        save.setBounds(5, 270, 63, 20);
        save.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
        save.onClick = [this]() {
            saveToFile.setVisible(true);
            saveToBackend.setVisible(true);
            cancelSave.setVisible(true);
            save.setVisible(false);
            load.setVisible(false);
            };
        renderProfile.addComponent(&save);

        load.setButtonText("Load");
        load.setBounds(73, 270, 63, 20);
        load.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgreen);
        load.onClick = [this]() {
			loadFromBackend.setVisible(true);
			loadFromFile.setVisible(true);
			cancelLoad.setVisible(true);
            save.setVisible(false);
            load.setVisible(false);

            // Update the list of loaded render states from the backend now while there is opportunity.
            renderStatesCached.clear();
            backenedListComboBox.clear();
            juce::Thread::launch([this]() {
                std::vector<struct RenderStateStruct> renderStates = getGetAllRenderStates(appSettings.getAuthJWT());
                juce::MessageManager::callAsync([this, renderStates]() {
                    for (auto renderState : renderStates) {
                        backenedListComboBox.addItem(renderState.name, renderState.id);
                        renderStatesCached.insert({ renderState.id, renderState });
                    }
                    });
                });
            };
        renderProfile.addComponent(&load);

        backenedListComboBox.setTextWhenNoChoicesAvailable("...");
        backenedListComboBox.setTextWhenNothingSelected("...");
        backenedListComboBox.setBounds(8, 240, 125, 25);
        backenedListComboBox.onChange = [this]() {
            auto rs = renderStatesCached.find(backenedListComboBox.getSelectedId());
            if (rs != renderStatesCached.end()) {
                struct RenderStateStruct renderState = rs->second;
                auto* fragShader = new juce::String(renderState.renderState);
                pendingFragShader.store(fragShader);
                pendingSubmit.store(true);
            }
            };
        renderProfile.addComponent(&backenedListComboBox);
        backenedListComboBox.setVisible(false);

        saveNameEditor.setText("My new visualiser...");
        saveNameEditor.setInputRestrictions(20);
        saveNameEditor.setBounds(8, 240, 125, 25);
        saveNameEditor.onReturnKey = [this]() {
            auto shaderPtr = std::atomic_load(&fragmentShader);
            juce::Thread::launch([this, shaderPtr]() {
                if (shaderPtr) {
                    juce::String name = saveNameEditor.getText();
                    juce::String newRSId = postAddRenderState(appSettings.getAuthJWT(), name, *shaderPtr);
                    DBG("Adding new render state id resolved from cloud as: " << newRSId);
                    juce::MessageManager::callAsync([this, name]() {
                        saveToBackend.setColour(juce::TextButton::ColourIds::buttonColourId, name.length() == 0 ? juce::Colours::lightcoral : juce::Colours::green);
                        });
                    delayColourChangeToComponent(&saveToBackend, juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
                }
                });
            saveNameEditor.setVisible(false);
            statusText.setVisible(true);
            };
        renderProfile.addComponent(&saveNameEditor);
        saveNameEditor.setVisible(false);

        // Type prompt logic here.
        prompt.setText("Type your prompt here!");
        prompt.setMultiLine(true, true);
        prompt.setReturnKeyStartsNewLine(true);
        prompt.setScrollbarsShown(true);
        prompt.setBounds(7, 7, 125, 187);
        renderProfile.addComponent(&prompt);
    }

    // Handle updating component entities on the messange thread. You can only update on the messange thread
    // and aquiring a MessageManagerLock on the render loop will block the GL thread until it aquires the lock.
    void handleAsyncUpdate() override {
        if (!appSettings.isAuth()) {
            statusText.setColour(juce::Label::textColourId, juce::Colours::red);
            statusText.setText("You must be logged-in in order to use this feature", juce::dontSendNotification);
            return;
        }
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

    void delayColourChangeToComponent(juce::TextButton* component, int colourId, juce::Colour colour) {
        juce::Thread::launch([component, colourId, colour]() {
            juce::Thread::sleep(2000);
            juce::MessageManager::callAsync([component, colourId, colour]() {
                if (component != nullptr)
                    component->setColour(colourId, colour);
                });
            });
    }


private:
    ApplicationSettings& appSettings;

    juce::TextButton loadFromFile;
    juce::TextButton loadFromBackend;
    juce::TextButton cancelLoad;
    juce::ComboBox backenedListComboBox;

    juce::TextButton saveToFile;
    juce::TextButton saveToBackend;
    juce::TextButton cancelSave;
    juce::TextEditor saveNameEditor;

    juce::Label statusText;
    juce::TextButton submit, save, load;
    juce::TextEditor prompt;
    juce::FileChooser saveChooser, loadChooser;

    std::atomic<juce::String*> pendingFragShader{ nullptr };
    std::atomic<bool> pendingAPIRequest{ false };
    std::atomic<bool> pendingSubmit{ false };
    std::atomic<bool> displayStatusError{ false };

    std::unordered_map<int, struct RenderStateStruct> renderStatesCached;
};