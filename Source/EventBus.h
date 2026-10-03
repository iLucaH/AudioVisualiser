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
    // Presets
    inline constexpr auto VisualiserPresetGetAll = "receive.preset.getall";
    inline constexpr auto VisualiserPresetSet = "receive.preset.set";
    inline constexpr auto VisualiserSubmitNewPrompt = "receive.preset.submit.prompt.new";

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
}

namespace Send_Events {
    inline constexpr auto VisualiserPresetChange = "send.visualiser.preset.change";
    inline constexpr auto VisualiserEffectChange = "send.visualiser.effect.changed";
    inline constexpr auto LoginComplete = "send.login.complete";
    inline constexpr auto RegisterComplete = "send.register.complete";

    inline constexpr std::array Events = {
        VisualiserPresetChange,
        VisualiserEffectChange,
        LoginComplete,
        RegisterComplete
    };
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