/*
  ==============================================================================

    EventBus.h
    Created: 3 Oct 2026 4:26:41pm
    Author:  lucas

    Init Usage:
        eventBus.subscribe(
            Receive_Events::GetVisualisers,
            [this](const juce::Array<juce::var>& args) {
                return getVisualisers();
            }
        );

    Deconstruct Usage:
        eventBus.unsubscribe(subscriptionID);

  ==============================================================================
*/

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <array>

namespace Receive_Events {

    // QR
    inline constexpr auto QROpenSite = "receive.qr.opensite";
    inline constexpr auto QRSocketHandle = "receive.qr.handle";

    // Presets
    inline constexpr auto VisualiserPresetGetAll = "receive.preset.getall";
    inline constexpr auto VisualiserPresetSet = "receive.preset.set";
    inline constexpr auto VisualiserPresetCurrent = "receive.preset.current";
    // Presets - AI
    inline constexpr auto VisualiserSubmitNewPrompt = "receive.preset.submit.prompt.new";
    inline constexpr auto VisualiserSaveToFile = "receive.preset.ai.save.file";
    inline constexpr auto VisualiserSaveToAccount = "receive.preset.ai.save.account";
    inline constexpr auto VisualiserLoadFromFile = "receive.preset.ai.load.file";
    inline constexpr auto VisualiserLoadFromAccountGet = "receive.preset.ai.load.account.get";
    inline constexpr auto VisualiserLoadFromAccountSet = "receive.preset.ai.load.account.set";
    // Presets - Image
    inline constexpr auto VisualiserImageLoad = "receive.preset.image.load";
    inline constexpr auto VisualiserImagePulse = "receive.preset.image.pulse";
    inline constexpr auto VisualiserImageShake = "receive.preset.image.shake";
    inline constexpr auto VisualiserImageRipple = "receive.preset.image.ripple";
    inline constexpr auto VisualiserImageRGBSplit = "receive.preset.image.rgbsplit";
    inline constexpr auto VisualiserImageGlitch = "receive.preset.image.glitch";
    inline constexpr auto VisualiserImageHueShift = "receive.preset.image.hueshift";
    inline constexpr auto VisualiserImageMirror = "receive.preset.image.mirror";
    inline constexpr auto VisualiserImageFlash = "receive.preset.image.flash";
    inline constexpr auto VisualiserImageIntensityUp = "receive.preset.image.intensity.up";
    inline constexpr auto VisualiserImageIntensityDown= "receive.preset.image.intensity.down";
    inline constexpr auto VisualiserImageReset = "receive.preset.image.reset";
    inline constexpr auto VisualiserImageFlipX = "receive.preset.image.flip.x";
    inline constexpr auto VisualiserImageFlipY = "receive.preset.image.flip.y";

    // Effects
    inline constexpr auto VisualiserEffectGetAll = "receive.effect.getall";
    inline constexpr auto VisualiserEffectPostProcessorSwitch = "receive.effect.postprocessor.enabled";
    inline constexpr auto VisualiserEffectUpdateEffect = "receive.effect.update";
    inline constexpr auto VisualiserEffectPostProcessorSwitchEnabled = "receive.effect.global.enabled";

    // Settings
    inline constexpr auto SettingsWidthGet = "settings.width.get";
    inline constexpr auto SettingsHeightGet = "settings.height.get";
    inline constexpr auto SettingsWidthSet = "settings.width.set";
    inline constexpr auto SettingsHeightSet = "settings.height.set";
    inline constexpr auto SettingsFullscreenGet = "settings.fullscreen.get";
    inline constexpr auto SettingsFullscreenSet = "settings.fullscreen.set";

    inline constexpr auto SettingsFFTSizeGet = "settings.fftsize.get";
    inline constexpr auto SettingsFFTSizeSet = "settings.fftsize.set";

    inline constexpr auto SettingsSocketPasswordGet = "settings.socketpassword.get";
    inline constexpr auto SettingsSocketPasswordSet = "settings.socketpassword.set";

    // Audio
    inline constexpr auto AudioSourceOpen = "audio.source.open";
    inline constexpr auto AudioPlay = "audio.play";
    inline constexpr auto AudioStop = "audio.stop";
    inline constexpr auto AudioSourceMasterScalarGet = "audio.source.master.scalar.get";
    inline constexpr auto AudioSourceMasterScalarSet = "audio.source.master.scalar.set";

    // Recording
    inline constexpr auto RecordingStart = "recording.start";
    inline constexpr auto RecordingStop = "recording.stop";
    inline constexpr auto RecordingStateGet = "recording.state";
    inline constexpr auto RecordingOutputpathGet = "recording.outputpath.get";
    inline constexpr auto RecordingOutputpathSet = "recording.outputpath.set";
    inline constexpr auto RecordingList = "recording.list";

    // Register
    struct Register {
        const char* handle;
        int args_ok;
        int invalid_username;
        int invalid_password;
    };
    inline constexpr Register RegisterNewUser = {
        "receive.register.new.user",
        0, 1, 2
    };

    // Login
    struct Login {
        const char* handle;
        int args_ok;
        int not_enough_args;
        int invalid_username;
        int invalid_password;
    };
    inline constexpr Login LoginUser = {
        "receive.login.user",
        0, 1, 2, 3,
    };
    inline constexpr auto LoginAuthTokenAlreadyExists = "receive.auth.token.already.exists";
}

namespace Send_Events {
    // Global
    inline constexpr auto GlobalSocketUpdateAll = "send.global.socket.update.all";

    inline constexpr auto PromptResponseComplete = "send.visualiser.submit.prompt.response.complete";
    inline constexpr auto VisualiserPresetChange = "send.visualiser.preset.change";
    inline constexpr auto VisualiserPresetNewPreset = "send.visualiser.preset.newpreset";
    inline constexpr auto VisualiserEffectChange = "send.visualiser.effect.changed";
    inline constexpr auto LoginComplete = "send.login.complete";
    inline constexpr auto RegisterComplete = "send.register.complete";
    inline constexpr auto SettingsUpdated = "send.settings.updated";

    inline constexpr std::array Events = {
        GlobalSocketUpdateAll,
        PromptResponseComplete,
        VisualiserPresetChange,
        VisualiserPresetNewPreset,
        LoginComplete,
        RegisterComplete,
        SettingsUpdated
    };
}

// Events that only take place inside the c++ backend.
namespace Local_Events {
    inline constexpr auto LoginComplete = "local.login.complete";
}

class EventBus {
public:

    using Callback = std::function<juce::var(const juce::Array<juce::var>&)>;

    using SubscriptionID = uint64_t;

    SubscriptionID subscribe(const juce::String& event, Callback callback) {
        const auto id = nextID++;

        listeners[event].push_back({id, std::move(callback)});

        return id;
    }

    void unsubscribe(SubscriptionID id) {
        for (auto& [event, callbacks] : listeners) {
            std::erase_if(callbacks, [id](const Listener& listener) {
                    return listener.id == id;
            });
        }
    }

    juce::var emit(const juce::String& event, const juce::Array<juce::var>& args = {}) {
        auto it = listeners.find(event);

        if (it == listeners.end())
            return {};

        for (auto& listener : it->second)
            return listener.callback(args);

        return {};
    }

private:

    struct Listener {
        SubscriptionID id;
        Callback callback;
    };

    std::unordered_map<juce::String, std::vector<Listener>> listeners;

    SubscriptionID nextID = 0;
};