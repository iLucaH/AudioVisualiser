/*
  ==============================================================================

    WebViewHelper.h
    Created: 6 Mar 2026 7:40:40pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

inline std::vector<std::byte> streamToVector(juce::InputStream& stream) {
    using namespace juce;
    const auto sizeInBytes = static_cast<size_t>(stream.getTotalLength());
    std::vector<std::byte> result(sizeInBytes);
    stream.setPosition(0);
    [[maybe_unused]] const auto bytesRead =
        stream.read(result.data(), result.size());
    jassert(bytesRead == static_cast<ssize_t>(sizeInBytes));
    return result;
}

inline const char* getMimeForExtension(const juce::String& extension) {
    static const std::unordered_map<juce::String, const char*> mimeMap = {
        {{"htm"}, "text/html"},
        {{"html"}, "text/html"},
        {{"txt"}, "text/plain"},
        {{"jpg"}, "image/jpeg"},
        {{"jpeg"}, "image/jpeg"},
        {{"svg"}, "image/svg+xml"},
        {{"ico"}, "image/vnd.microsoft.icon"},
        {{"json"}, "application/json"},
        {{"png"}, "image/png"},
        {{"css"}, "text/css"},
        {{"map"}, "application/json"},
        {{"js"}, "text/javascript"},
        {{"woff2"}, "font/woff2"} };

    if (const auto it = mimeMap.find(extension.toLowerCase());
        it != mimeMap.end())
        return it->second;

    jassertfalse;
    return "";
}

inline auto getResource(const juce::String& url) -> std::optional<juce::WebBrowserComponent::Resource> {
    static const auto resourceFileRoot = juce::File{R"(C:\Users\lucas\Documents\JuceProjects\AudioVisualiser\ui\public)"}; // juce::File::getCurrentWorkingDirectory()
    const auto resourceToRetrieve = url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);
    const auto resource = resourceFileRoot.getChildFile(resourceToRetrieve).createInputStream();
    DBG("Loading WebView resource: " << resourceFileRoot.getChildFile(resourceToRetrieve).getFullPathName());
    if (resource) {
        const auto extension = resourceToRetrieve.fromLastOccurrenceOf(".", false, false);
        return juce::WebBrowserComponent::Resource{streamToVector(*resource), getMimeForExtension(extension)};
    }
    return std::nullopt;
}