/*
  ==============================================================================

    AVAPIResolver.h
    Created: 18 Feb 2026 4:35:31pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Blocking operation.
inline juce::String getResultText(const juce::URL& url) {
    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(120000);;
    auto stream = url.createInputStream(options);
    // Stream may timeout or the service may be unreachable.
    if (stream == nullptr) {
        return "";
    }

    return stream->readEntireStreamAsString();
}

// Blocking operation.
inline juce::String getPromptResponse(const juce::String& prompt) {
    juce::URL url("http://localhost:8080/prompt");
    url = url.withParameter("token", "1234");
    url = url.withParameter("prompt", prompt);
    DBG("Calling URL: " << url.toString(true));
    juce::String response = getResultText(url);
    if (response.length() == 0) {
        DBG("Failed to receive an API JSON Prompt Response!");
        return "";
    }

    juce::var parsed = juce::JSON::parse(response);
    if (parsed.isVoid()) {
        DBG("Failed to parse API JSON Prompt Response!");
        return "";
    }
    juce::DynamicObject* obj = parsed.getDynamicObject();
    if (obj == nullptr) {
        DBG("API JSON Prompt Response is a nullptr!");
        return "";
    }
    if (obj->getProperty("success").isVoid()) {
        DBG("No success status could be resolved from API JSON Prompt Response!");
        return "";
    }
    if (!obj->getProperty("success")) {
        DBG("A failed status was resolved from API JSON Prompt Response!");
        return "";
    }
    if (obj->getProperty("prompt").isVoid()) {
        DBG("No success status could be resolved from API JSON Prompt Response!");
        return "";
    }
    juce::String parsedResponse = obj->getProperty("prompt").toString();
    DBG("API JSON Prompt Response resolved to: " << parsedResponse);
    return parsedResponse;
}