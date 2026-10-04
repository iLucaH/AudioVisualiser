/*
  ==============================================================================

    SelectorTabPanel.cpp
    Created: 19 Nov 2025 8:24:33am
    Author:  lucas

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SelectorTabPanel.h"
#include "RenderState2D.h"
#include "PluginEditor.h"

//==============================================================================
SelectorTabPanel::SelectorTabPanel(AudioVisualiserAudioProcessor& p, OpenGLComponent& openGL, ApplicationSettings& applSettings) : pluginProcessor(p), openGLComponent(openGL), appSettings(applSettings), settingsComponent(applSettings), appQRComponent(applSettings),
    openChooser("Choose a Wav or AIFF File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.mp3") {
    // Render state logic
    presetSelector.setHelpText("Click here to select a render state!");
    presetSelector.setTextWhenNothingSelected("Select Preset");
    presetSelector.setBounds(8, 8, 123, 25);
    presetSelector.onChange = [this]() {
        if (openGLComponent.isFullScreen())
            return;
        int newState = presetSelector.getSelectedId();
        updatePanelRenderProfile(newState, selectedState);
        selectedState = newState;
        };
    addAndMakeVisible(&presetSelector);
    
    for (int i = 0; i < openGL.getNumRenderStates(); i++) {
        addRenderPofile(openGL.getProfileComponent(i)); // Here they will be added to the presetSelector.
        openGL.getProfileComponent(i)->subscribeToEvents(eventBus);
    }
    presetSelector.setSelectedId(DEFAULT_RENDER_STATE);

    openInApp.setButtonText("Open In App");
    openInApp.setBounds(8, 40, 123, 25);
    openInApp.onClick = [this] {
        if (openGLComponent.isFullScreen())
            return;
        DBG("Launching the open in app panel!");
        appQRComponent.addToDesktop();
        appQRComponent.setVisible(true);
        appQRComponent.toFront(true);
        };
    addAndMakeVisible(openInApp);

    fullscreen.setButtonText("Fullscreen");
    fullscreen.setBounds(8, 72, 123, 25);
    fullscreen.onClick = [this] {
        if (openGLComponent.isFullScreen())
            return;
        appSettings.setFullScreen(true);
        };
    addAndMakeVisible(fullscreen);

    open.setButtonText("Open");
    open.setBounds(6, 104, 40, 25);
    open.onClick = [this] {
        if (openGLComponent.isFullScreen())
            return;
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        openChooser.launchAsync(flags, [this](const juce::FileChooser& chooser) {
            juce::File file = chooser.getResult();
            DBG("File selected for playback!");
            pluginProcessor.setNewTransportSource(file);
            });
        };
    addAndMakeVisible(open);

    play.setButtonText("Play");
    play.setBounds(50, 104, 40, 25);
    play.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
    play.onClick = [this] {
        if (openGLComponent.isFullScreen())
            return;
        processPlay();
        };
    addAndMakeVisible(play);

    stop.setButtonText("Stop");
    stop.setBounds(94, 104, 40, 25);
    stop.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::palevioletred);
    stop.onClick = [this] {
        if (openGLComponent.isFullScreen())
            return;
        processStop();
        };
    addAndMakeVisible(stop);

    settings.setButtonText("Settings");
    settings.setBounds(8, 136, 123, 25);
    settings.onClick = [this] {
        if (openGLComponent.isFullScreen())
            return;
        DBG("Launching the login panel!");
        settingsComponent.addToDesktop();
        settingsComponent.setVisible(true);
        settingsComponent.toFront(true);
        };
    addAndMakeVisible(settings);

    // Event Bus Listeners for Panel

    // For when React asks for all render states.
    eventBus.subscribe(Receive_Events::VisualiserPresetGetAll,
        [this](const auto& args) {
            juce::Array<juce::var> presets;
            for (auto* profile : renderProfiles) {
                presets.add(profile->getFrontEndPresets());
            }
            return juce::var(presets);
        }
    );

    eventBus.subscribe(Receive_Events::VisualiserPresetCurrent,
        [this](const auto& args) {
            return juce::var(static_cast<int>(selectedState));
        }
    );

    // For when React tells us to change render state.
    eventBus.subscribe(Receive_Events::VisualiserPresetSet,
        [this](const auto& args) {
            if (args.size() < 2) {
                return false;
            }
            int newState = static_cast<int>(args[1]);
            if (newState <= 0 || newState > renderProfiles.size()) {
                return false;
            }
            updatePanelRenderProfile(newState, selectedState);
            selectedState = newState;
            return true;
        }
    );

    // Event bus listeners for login/register

    eventBus.subscribe(Receive_Events::RegisterNewUser.handle,
        [this](const auto& args) {
            if (args.size() < 3) {
                return Receive_Events::RegisterNewUser.invalid_username; // good enough i guess
            }
            return appSettings.getRoot()->getLoginComponent().getContentComponent()->registerNewUser(args[1], args[2]);
        }
    );

    eventBus.subscribe(Receive_Events::LoginUser.handle,
        [this](const auto& args) {
            if (args.size() < 3) {
                return Receive_Events::LoginUser.not_enough_args;
            }
            return appSettings.getRoot()->getLoginComponent().getContentComponent()->loginUser(args[1], args[2]);
        }
    );

    eventBus.subscribe(Receive_Events::LoginAuthTokenAlreadyExists,
        [this](const auto& args) {
            if (args.size() < 2) {
                return juce::var();
            }
            eventBus.emit(Local_Events::LoginComplete, juce::var());
            appSettings.setAuthJWT(args[1]);
            return juce::var();
        }
    );

    // Event bus listeners for settings

    eventBus.subscribe(Receive_Events::SettingsWidthGet,
        [this](const auto& args) {
            return juce::var(appSettings.getWidth());
        }
    );

    eventBus.subscribe(Receive_Events::SettingsWidthSet,
        [this](const auto& args) {
            if (args.size() < 2) {
                return juce::var(false);
            }
            appSettings.setDimensions(static_cast<int>(args[1]), appSettings.getHeight());
            return juce::var(true);
        }
    );

    eventBus.subscribe(Receive_Events::SettingsHeightGet,
        [this](const auto& args) {
            return juce::var(appSettings.getHeight());
        }
    );

    eventBus.subscribe(Receive_Events::SettingsHeightSet,
        [this](const auto& args) {
            if (args.size() < 2) {
                return juce::var(false);
            }
            appSettings.setDimensions(appSettings.getWidth(), static_cast<int>(args[1]));
            return juce::var(true);
        }
    );

    eventBus.subscribe(Receive_Events::SettingsFullscreenGet,
        [this](const auto& args) {
            return juce::var(openGLComponent.isFullScreen());
        }
    );

    eventBus.subscribe(Receive_Events::SettingsFullscreenSet,
        [this](const auto& args) {
            if (args.size() < 2) {
                return juce::var(false);
            }
            appSettings.setFullScreen(!openGLComponent.isFullScreen());
            return juce::var(true);
        }
    );

    eventBus.subscribe(Receive_Events::SettingsFFTSizeGet,
        [this](const auto& args) {
            return juce::var(appSettings.getFFTSize());
        }
    );

    eventBus.subscribe(Receive_Events::SettingsFFTSizeSet,
        [this](const auto& args) {
            if (args.size() < 2) {
                return juce::var(false);
            }
            appSettings.setFFTSize(static_cast<int>(args[1]));
            return juce::var(true);
        }
    );

    eventBus.subscribe(Receive_Events::SettingsSocketPasswordGet,
        [this](const auto& args) {
            return juce::var(appSettings.getSocketClientAuth());
        }
    );

    eventBus.subscribe(Receive_Events::SettingsSocketPasswordSet,
        [this](const auto& args) {
            if (args.size() < 2) {
                return juce::var(false);
            }
            appSettings.setSocketClientAuth(args[1]);
            return juce::var(true);
        }
    );

    // audio

    eventBus.subscribe(Receive_Events::AudioSourceOpen,
        [this](const auto& args) {
            auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
            openChooser.launchAsync(flags, [this](const juce::FileChooser& chooser) {
                juce::File file = chooser.getResult();
                DBG("File selected for playback!");
                pluginProcessor.setNewTransportSource(file);
                });
            return juce::var();
        }
    );

    eventBus.subscribe(Receive_Events::AudioPlay,
        [this](const auto& args) {
            processPlay();
            return juce::var();
        }
    );

    eventBus.subscribe(Receive_Events::AudioStop,
        [this](const auto& args) {
            processStop();
            return juce::var();
        }
    );

    eventBus.subscribe(Receive_Events::AudioSourceMasterScalarGet,
        [this](const auto& args) {
            return juce::var(appSettings.getAudioScalar());
        }
    );

    eventBus.subscribe(Receive_Events::AudioSourceMasterScalarSet,
        [this](const auto& args) {
            if (args.size() < 2) {
                return juce::var(false);
            }
            appSettings.setAudioScalar(static_cast<float>(args[1]));
            return juce::var(true);
        }
    );

    // QR 

    eventBus.subscribe(Receive_Events::QROpenSite,
        [this](const auto& args) {
            if (args.size() > 1) {
                juce::URL(args[1]).launchInDefaultBrowser();
            }
            return juce::var();
        }
    );
}

SelectorTabPanel::~SelectorTabPanel(){
}

void SelectorTabPanel::paint (juce::Graphics& g) {
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::white);
    g.drawRect (getLocalBounds(), 1);
}

void SelectorTabPanel::resized() {
    auto area = getLocalBounds();
    for (auto* profile : renderProfiles)
        profile->setBounds(juce::Rectangle(0, 168, 140, 298));
}

void SelectorTabPanel::processPlay() {
    DBG("Audio Transport State is being changed to Playing by the play button in selector tab panel.");
    pluginProcessor.transportStateChanged(AudioVisualiserAudioProcessor::TransportState::Starting);
}

void SelectorTabPanel::processStop() {
    DBG("Audio Transport State is being changed to Stopping by the stop button in selector tab panel.");
    pluginProcessor.transportStateChanged(AudioVisualiserAudioProcessor::TransportState::Stopping);
}

void SelectorTabPanel::processRenderStateIncrement() {
    int newState = 1 + (presetSelector.getSelectedId() + 1 % presetSelector.getNumItems());
    presetSelector.setSelectedId(newState);
    updatePanelRenderProfile(newState, selectedState);
    selectedState = newState;
    openGLComponent.setSelectedState(selectedState);
}

void SelectorTabPanel::processRenderStateDecrement() {
    int newState = presetSelector.getSelectedId() - 1;
    if (newState < 1)
        newState = presetSelector.getNumItems();
    presetSelector.setSelectedId(newState);
    updatePanelRenderProfile(newState, selectedState);
    selectedState = newState;
    openGLComponent.setSelectedState(selectedState);
}