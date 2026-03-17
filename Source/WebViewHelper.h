/*
  ==============================================================================

    WebViewHelper.h
    Created: 6 Mar 2026 7:40:40pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <ranges>
#include <WebViewFiles.h>

/*
    Normalise paths to user the / file seperator convention. 
*/
inline juce::String normalizePath(const juce::String& path) {
    return path.replaceCharacter('\\', '/').toLowerCase();
}

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

inline std::vector<std::byte> getWebViewFilesAsBytes(const juce::String resourceToRetrieve) {
    juce::MemoryInputStream zipStream{ webview_files::ui_zip, webview_files::ui_zipSize, false };
    juce::ZipFile zipFile{ zipStream };

    for (const auto i : std::views::iota(0, zipFile.getNumEntries())) {
        const auto* zipEntry = zipFile.getEntry(i);

        if (normalizePath(zipEntry->filename) == normalizePath(resourceToRetrieve)) {
            DBG("The path " << normalizePath(zipEntry->filename) << " was found when trying to retrieve " << resourceToRetrieve);
            const std::unique_ptr<juce::InputStream> entryStream{ zipFile.createStreamForEntry(*zipEntry) };
            return streamToVector(*entryStream);
        }
    }
    return {};
}

inline auto getResource(const juce::String& url) -> std::optional<juce::WebBrowserComponent::Resource> {

    const auto resourceToRetrieve = url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

    const auto resource = getWebViewFilesAsBytes(resourceToRetrieve);
    if (!resource.empty()) {
        const auto extension = resourceToRetrieve.fromLastOccurrenceOf(".", false, false);
        return juce::WebBrowserComponent::Resource{std::move(resource), getMimeForExtension(extension)};
    }
    return std::nullopt;
}