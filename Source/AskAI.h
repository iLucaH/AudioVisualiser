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
#include "SelectorTabPanel.h"
#include "EventBus.h"

class AskAI : public RenderState2D, public juce::AsyncUpdater {
public:
    AskAI(int id, OpenGLComponent& glComponent, juce::OpenGLContext& context, ApplicationSettings& applSettings) : openGLComponent(glComponent), openGLContext(context), RenderState2D(id, context, juce::String(R"(
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
)")), appSettings(applSettings),
saveChooser("Save Shader", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.avrs"),
loadChooser("Load Shader", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.avrs") {
        renderProfile.setPresetName("AI Generator");
        renderProfile.setFrontEndPresets(createPresetSettings());

        saveEnterTitleText.setText("Enter name:", juce::dontSendNotification);
        saveEnterTitleText.setBorderSize(juce::BorderSize<int>(2));
        saveEnterTitleText.setBounds(6, 147, 125, 125);
        renderProfile.addComponent(&saveEnterTitleText);
        saveEnterTitleText.setVisible(false);

        selectBackenedRenderStateText.setText("Select from cloud:", juce::dontSendNotification);
        selectBackenedRenderStateText.setBorderSize(juce::BorderSize<int>(2));
        selectBackenedRenderStateText.setBounds(6, 167, 125, 125);
        renderProfile.addComponent(&selectBackenedRenderStateText);
        selectBackenedRenderStateText.setVisible(false);

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
                bool success = response.length() > 0;
                if (success) {
                    auto* fragShader = new juce::String(response); // The fragShader will be freed once exchanged in the render loop.
                    pendingFragShader.store(fragShader);
                    auto* fragShaderName = new juce::String("My New Shader");
                    pendingFragShaderName.store(fragShaderName);
                    pendingSubmit.store(true);
                    DBG("Resolved a prompt for the AskAI RenderState!");
                } else {
                    DBG("Could not resolve a prompt for the AskAI RenderState!");
                    displayStatusError.store(true);
                }
                pendingAPIRequest.store(false);
                juce::MessageManager::callAsync([this, success]() {
                    juce::Array<juce::var> response;
                    response.add(success);
                    response.add(success ? "Everything worked!" : "There was an error processing your prompt!");
                    appSettings.getEventBus().emit(Send_Events::PromptResponseComplete, juce::var(response));
                });
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
                auto* fragShaderName = new juce::String(chooser.getResult().getFileName());
                pendingFragShaderName.store(fragShaderName);
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
                loadFromBackend.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightcoral);
                delayColourChangeToComponent(&loadFromBackend, juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
                return;
            }
            backenedListComboBox.setVisible(true);
            selectBackenedRenderStateText.setVisible(true);
            statusText.setVisible(false);
            submit.setVisible(false);
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
            selectBackenedRenderStateText.setVisible(false);
            save.setVisible(true);
            load.setVisible(true);
            statusText.setVisible(true);
            submit.setVisible(true);
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
            submit.setVisible(false);
            statusText.setVisible(false); // Hide status text so we can make room for the text box.
            saveNameEditor.setVisible(true);
            confirmSave.setVisible(true);
            saveEnterTitleText.setVisible(true);
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
            confirmSave.setVisible(false);
            saveEnterTitleText.setVisible(false);
            save.setVisible(true);
            load.setVisible(true);
            statusText.setVisible(true);
            submit.setVisible(true);
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
            juce::Thread::launch([this]() {
                std::vector<struct RenderStateStruct> renderStates = getGetAllRenderStates(appSettings.getAuthJWT());
                juce::MessageManager::callAsync([this, renderStates]() {
                    for (auto renderState : renderStates) {
                        bool exists = false;
                        for (int i = 0; i < backenedListComboBox.getNumItems(); i++) {
                            if (backenedListComboBox.getItemText(i).equalsIgnoreCase(renderState.name)) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            backenedListComboBox.addItem(renderState.name, renderState.id);
                            renderStatesCached.insert({ renderState.id, renderState });
                        }
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
                auto* fragShaderName = new juce::String(renderState.name);
                pendingFragShaderName.store(fragShaderName);
                pendingSubmit.store(true);
            }
            };
        renderProfile.addComponent(&backenedListComboBox);
        backenedListComboBox.setVisible(false);

        saveNameEditor.setText("My new visualiser...");
        saveNameEditor.setInputRestrictions(20);
        saveNameEditor.setBounds(8, 219, 125, 25);
        saveNameEditor.onReturnKey = [this]() {
            auto shaderPtr = std::atomic_load(&fragmentShader);
            confirmSaveShaderToBackend();
            saveNameEditor.setVisible(false);
            confirmSave.setVisible(false);
            saveEnterTitleText.setVisible(false);
            statusText.setVisible(true);
            };
        renderProfile.addComponent(&saveNameEditor);
        saveNameEditor.setVisible(false);

        confirmSave.setButtonText("Confirm");
        confirmSave.setBounds(20, 246, 100, 20);
        confirmSave.onClick = [this]() {
            confirmSaveShaderToBackend();
            saveNameEditor.setVisible(false);
            confirmSave.setVisible(false);
            saveEnterTitleText.setVisible(false);
            statusText.setVisible(true);
            };
        renderProfile.addComponent(&confirmSave);
        confirmSave.setVisible(false);

        // Type prompt logic here.
        prompt.setText("Type your prompt here!");
        prompt.setMultiLine(true, true);
        prompt.setReturnKeyStartsNewLine(true);
        prompt.setScrollbarsShown(true);
        prompt.setBounds(7, 7, 125, 187);
        renderProfile.addComponent(&prompt);

        renderProfile.setEventSubscription(
            [this](EventBus& eventBus) {
                eventBus.subscribe(Local_Events::LoginComplete, // local handler to know when a login is successful from login component.
                    [this](const auto& args) {
                        handleLoginPolls();
                        return juce::var();
                    }
                );

                eventBus.subscribe(Receive_Events::VisualiserSubmitNewPrompt,
                    [this](const auto& args) {
                        if (args.size() < 2) {
                            return juce::var(juce::Array<juce::var>{ false, "Please write a prompt!" });
                        }
                        juce::String promptResponse = args[1];
                        return juce::var(handleSubmit(promptResponse));
                    }
                );

                eventBus.subscribe(Receive_Events::VisualiserSaveToFile, // always returns true because it will always launch.
                    [this](const auto& args) {
                        bool success = handleSaveToFile();
                        return juce::var(juce::String(success ? "" : "Error saving to file!"));
                    }
                );

                eventBus.subscribe(Receive_Events::VisualiserLoadFromFile, // always returns true because it will always launch.
                    [this](const auto& args) {
                        bool success = handleLoadFromFile();
                        handleLoginPolls();
                        return juce::var(juce::String(success ? "" : "Error loading from file!"));
                    }
                );

                eventBus.subscribe(Receive_Events::VisualiserLoadFromAccountGet,
                    [this](const auto& args) {
                        handleLoginPolls();
                        return juce::var(handleLoadFromAccountGet());
                    }
                );

                eventBus.subscribe(Receive_Events::VisualiserLoadFromAccountSet,
                    [this](const auto& args) {
                        if (args.size() < 2) {
                            return juce::var(false);
                        }
                        if (!appSettings.isAuth()) {
                            return juce::var(false);
                        }
                        return juce::var(handleLoadFromAccountSet(static_cast<int>(args[1])));
                    }
                );

                eventBus.subscribe(Receive_Events::VisualiserSaveToAccount,
                    [this](const auto& args) {
                        if (args.size() < 2) {
                            return juce::var(juce::String("Please enter a name."));
                        }
                        if (!appSettings.isAuth()) {
                            return juce::var(juce::String("You must be logged in."));
                        }
                        return juce::var(juce::String(handleSaveToAccount(args[1]) ? "Succcessfully saved your preset!" : "Failed to save preset!"));
                    }
                );

                // eventBus.subscribe(...);
            }
        );
    }

    bool handleLoadFromAccountSet(int id) {
        auto rs = renderStatesCached.find(id);
        if (rs != renderStatesCached.end()) {
            struct RenderStateStruct renderState = rs->second;
            auto* fragShader = new juce::String(renderState.renderState);
            pendingFragShader.store(fragShader);
            auto* fragShaderName = new juce::String(renderState.name);
            pendingFragShaderName.store(fragShaderName);
            pendingSubmit.store(true);
            shouldCreateNewRenderState.store(true);
            handleLoginPolls();
            return true;
        }
        return false;
    }

    bool handleSaveToAccount(juce::String name) {
        auto shaderPtr = std::atomic_load(&fragmentShader);
        juce::Thread::launch([this, shaderPtr, name]() {
            if (shaderPtr) {
                juce::String newRSId = postAddRenderState(appSettings.getAuthJWT(), name, *shaderPtr);
                DBG("Adding new render state id resolved from cloud as: " << newRSId);
            }
        });
        pendingFragShaderName.store(new juce::String(name));
        shouldCreateNewRenderState.store(true);
        return true;
    }

    juce::Array<juce::var> handleLoadFromAccountGet() {
        juce::Array<juce::var> presets;
        if (appSettings.isAuth()) {
            for (const auto& [id, renderState] : renderStatesCached) {
                auto* object = new juce::DynamicObject();
                object->setProperty("value", renderState.name);
                object->setProperty("key", juce::String(id));
                presets.add(juce::var(object));
            }
        } else {
            auto* object = new juce::DynamicObject();
            object->setProperty("value", "Please log-in...");
            object->setProperty("key", -1);
            presets.add(juce::var(object));
        }
        return presets;
    }

    bool handleSaveToFile() {
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
        return true;
    }

    bool handleLoadFromFile() {
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        loadChooser.launchAsync(flags, [this](const juce::FileChooser& chooser) {
            juce::String filePath = chooser.getResult().getFullPathName();
            if (filePath.isEmpty())
                return;
            DBG("Loading render state from " << filePath);
            juce::String renderState = getRenderStateFromFile(filePath);
            auto* fragShader = new juce::String(renderState);
            pendingFragShader.store(fragShader);
            auto* fragShaderName = new juce::String(chooser.getResult().getFileName());
            pendingFragShaderName.store(fragShaderName);
            pendingSubmit.store(true);
            shouldCreateNewRenderState.store(true);
        });
        return true;
    }

    juce::Array<juce::var> handleSubmit(const juce::String& promptText) {
        juce::Array<juce::var> response;
        // If we are already making a prompt submit request, then we should not continue with this one.
        if (pendingAPIRequest.load()) {
            response.add(false);
            response.add("Please wait for the previous prompt to finish!");
            return response;
        } else if (!appSettings.isAuth()) {
            response.add(false);
            response.add("Please log in to be able to use this feature!");
            return response;
        }

        // Launch the API request on a seperate thread because it is a blocking operation.
        juce::Thread::launch([this, promptText]() {
            pendingAPIRequest.store(true);
            juce::String response = postPromptResponse(appSettings.getAuthJWT(), promptText);
            bool success = response.length() > 0;
            if (success) {
                auto* fragShader = new juce::String(response); // The fragShader will be freed once exchanged in the render loop.
                pendingFragShader.store(fragShader);
                auto* fragShaderName = new juce::String("My New Shader");
                pendingFragShaderName.store(fragShaderName);
                pendingSubmit.store(true);
                DBG("Resolved a prompt for the AskAI RenderState!");
            } else {
                DBG("Could not resolve a prompt for the AskAI RenderState!");
                displayStatusError.store(true);
            }
            pendingAPIRequest.store(false);
            juce::MessageManager::callAsync([this, success]() { // inform the front end of the changes
                juce::Array<juce::var> responseAsync;
                responseAsync.add(success);
                responseAsync.add(success ? "" : "There was an error processing your prompt!");
                appSettings.getEventBus().emit(Send_Events::PromptResponseComplete, juce::var(responseAsync));
            });
        });
        response.add(true);
        response.add("Loading Shader...");
        return response;
    }

    void handleLoginPolls() {
        juce::Thread::launch([this]() {
            std::vector<struct RenderStateStruct> renderStates = getGetAllRenderStates(appSettings.getAuthJWT());
            juce::MessageManager::callAsync([this, renderStates]() {
                for (auto renderState : renderStates) {
                    bool exists = false;
                    for (int i = 0; i < backenedListComboBox.getNumItems(); i++) {
                        if (backenedListComboBox.getItemText(i).equalsIgnoreCase(renderState.name)) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        backenedListComboBox.addItem(renderState.name, renderState.id);
                        renderStatesCached.insert({ renderState.id, renderState });
                    }
                }
            });
        });
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
        }
        else if (displayStatusError.load()) {
            statusText.setColour(juce::Label::textColourId, juce::Colours::red);
            statusText.setText("There was an error\nloading the shader!", juce::dontSendNotification);
        }
        else {
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

                if (shouldCreateNewRenderState.exchange(false)) {
                    addNewRenderStateFromLoadOrSave();
                }

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

    void addNewRenderStateFromLoadOrSave() {
        juce::String* shaderNamePtr = pendingFragShaderName.exchange(nullptr);

        // Make a copy while the pointer is still valid.
        juce::String shaderName = shaderNamePtr != nullptr ? *shaderNamePtr : "Unnamed Render State";

        delete shaderNamePtr;

        // Before going any further lets check if another shader with this name has already added,
        // because we don't want to add it twice.
        if (openGLComponent.renderStateExistsByName(shaderName)) {
            return;
        }

        juce::String fragmentShaderCopy = fragmentShader != nullptr ? *fragmentShader : "";

        std::unique_ptr<RenderState> newRenderState = std::make_unique<RenderState2D>(
            openGLComponent.getNextAvailableRenderStateID(),
            openGLContext,
            juce::String(R"(
                #version 330 core
                layout(location = 0) in vec4 position;

                void main() {
                    gl_Position = position;
                }
            )"), fragmentShaderCopy);

        newRenderState->initAndCompileShaders();

        newRenderState->getRenderProfile()->setPresetName(shaderName);

        openGLComponent.addRenderState(std::move(newRenderState));
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

    void confirmSaveShaderToBackend() {
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
        submit.setVisible(true);
    }

private:
    ApplicationSettings& appSettings;
    OpenGLComponent& openGLComponent;
    juce::OpenGLContext& openGLContext;

    juce::TextButton loadFromFile;
    juce::TextButton loadFromBackend;
    juce::TextButton cancelLoad;
    juce::ComboBox backenedListComboBox;

    juce::TextButton saveToFile;
    juce::TextButton saveToBackend;
    juce::TextButton cancelSave;
    juce::TextButton confirmSave;
    juce::TextEditor saveNameEditor;
    juce::Label saveEnterTitleText;
    juce::Label selectBackenedRenderStateText;

    juce::Label statusText;
    juce::TextButton submit, save, load;
    juce::TextEditor prompt;
    juce::FileChooser saveChooser, loadChooser;

    std::atomic<juce::String*> pendingFragShader{ nullptr };
    std::atomic<juce::String*> pendingFragShaderName{ nullptr };
    std::atomic<bool> pendingAPIRequest{ false };
    std::atomic<bool> pendingSubmit{ false };
    std::atomic<bool> displayStatusError{ false };
    std::atomic<bool> shouldCreateNewRenderState{ false };

    std::unordered_map<int, struct RenderStateStruct> renderStatesCached;

    juce::var createPresetSettings() {
        auto aiGenerator = new juce::DynamicObject();
        aiGenerator->setProperty("value", renderProfile.getPresetName());
        aiGenerator->setProperty("key", renderProfile.getRenderStateID());

        juce::Array<juce::var> aiContent;

        // Prompt
        auto prompt = new juce::DynamicObject();
        prompt->setProperty("type", "textfieldlong");
        prompt->setProperty("label", "Prompt");
        prompt->setProperty("clickhandler", Receive_Events::VisualiserSubmitNewPrompt);
        aiContent.add(juce::var(prompt));

        // Generate Design
        auto generate = new juce::DynamicObject();
        generate->setProperty("type", "waitingbutton");
        generate->setProperty("label", "Generate Design");
        generate->setProperty("clickhandler", Receive_Events::VisualiserSubmitNewPrompt);
        generate->setProperty("waitingpromisehandle", Send_Events::PromptResponseComplete);
        generate->setProperty("link", "Prompt");
        aiContent.add(juce::var(generate));

        // Spacer
        auto spacer = new juce::DynamicObject();
        spacer->setProperty("type", "spacer");
        spacer->setProperty("paddingTop", 15);
        spacer->setProperty("paddingBottom", 15);
        aiContent.add(juce::var(spacer));

        // ================================================================
        // Row
        // ================================================================

        auto row = new juce::DynamicObject();
        row->setProperty("type", "row");

        juce::Array<juce::var> rowContent;

        // ================================================================
        // Save Preset
        // ================================================================

        auto savePreset = new juce::DynamicObject();
        savePreset->setProperty("type", "expandbutton");
        savePreset->setProperty("label", "Save Preset");

        juce::Array<juce::var> saveContent;

        // To File
        auto toFile = new juce::DynamicObject();
        toFile->setProperty("type", "button");
        toFile->setProperty("label", "To File");
        toFile->setProperty("clickhandler", Receive_Events::VisualiserSaveToFile);
        toFile->setProperty("link", "Prompt");
        saveContent.add(juce::var(toFile));

        // To Account
        auto toAccount = new juce::DynamicObject();
        toAccount->setProperty("type", "expandbutton");
        toAccount->setProperty("label", "To Account");

        juce::Array<juce::var> accountContent;

        // Preset Name
        auto presetName = new juce::DynamicObject();
        presetName->setProperty("type", "textfieldshort");
        presetName->setProperty("label", "Name your preset");
        presetName->setProperty("clickhandler", Receive_Events::VisualiserSaveToAccount);
        accountContent.add(juce::var(presetName));

        // Save
        auto save = new juce::DynamicObject();
        save->setProperty("type", "button");
        save->setProperty("label", "Save");
        save->setProperty("clickhandler", Receive_Events::VisualiserSaveToAccount);
        save->setProperty("link", "Name your preset");
        accountContent.add(juce::var(save));

        toAccount->setProperty("subcontent", accountContent);
        saveContent.add(juce::var(toAccount));

        savePreset->setProperty("subcontent", saveContent);
        rowContent.add(juce::var(savePreset));

        // ================================================================
        // Load Preset
        // ================================================================

        auto loadPreset = new juce::DynamicObject();
        loadPreset->setProperty("type", "expandbutton");
        loadPreset->setProperty("label", "Load Preset");

        juce::Array<juce::var> loadContent;

        // From File
        auto fromFile = new juce::DynamicObject();
        fromFile->setProperty("type", "button");
        fromFile->setProperty("label", "From File");
        fromFile->setProperty("clickhandler", Receive_Events::VisualiserLoadFromFile);
        fromFile->setProperty("link", "Prompt");
        loadContent.add(juce::var(fromFile));

        // From Account
        auto fromAccount = new juce::DynamicObject();
        fromAccount->setProperty("type", "expandbutton");
        fromAccount->setProperty("label", "From Account");

        juce::Array<juce::var> accountLoadContent;

        // Account Picker
        auto picker = new juce::DynamicObject();
        picker->setProperty("type", "picker");
        picker->setProperty("label", "Select Preset...");
        picker->setProperty("gethandle", Receive_Events::VisualiserLoadFromAccountGet);
        picker->setProperty("sethandle", Receive_Events::VisualiserLoadFromAccountSet);

        accountLoadContent.add(juce::var(picker));

        fromAccount->setProperty("subcontent", accountLoadContent);
        loadContent.add(juce::var(fromAccount));

        loadPreset->setProperty("subcontent", loadContent);
        rowContent.add(juce::var(loadPreset));

        // ================================================================

        row->setProperty("subcontent", rowContent);
        aiContent.add(juce::var(row));

        aiGenerator->setProperty("subcontent", aiContent);

        return juce::var(aiGenerator);
    }
};