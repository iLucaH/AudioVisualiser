/*
  ==============================================================================

	AppQRComponent.h
	Created: 17 Mar 2026 17:05:11pm
	Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Settings.h"
#include "WebViewHelper.h"

class AppQRContentComponent : public juce::Component {
public:
	AppQRContentComponent(ApplicationSettings& appSettings) :
		settings(appSettings),
		webView{ juce::WebBrowserComponent::Options{}
			.withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
			.withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
				.withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory))
				.withBackgroundColour(juce::Colours::white))
			.withResourceProvider([this](const auto& url) { return getResource(url); })
			.withNativeFunction(juce::Identifier{"nativeFunctionGetSocketHandle"}, [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
				nativeFunctionGetSocketHandle(std::move(completion));
				})
			.withNativeIntegrationEnabled() } {

		webView.goToURL(webView.getResourceProviderRoot() + "appqr.html");
		DBG("WebView Location set to Root: " << webView.getResourceProviderRoot());

		addAndMakeVisible(webView);
	}

	void resized() override {
		webView.setBounds(getLocalBounds()); // Make the web view fit the entire window on resize.
	}

private:
	ApplicationSettings& settings;
	juce::WebBrowserComponent webView;

	void nativeFunctionGetSocketHandle(juce::WebBrowserComponent::NativeFunctionCompletion completion) {
		completion(settings.getSocketConnectionHandle());
	}
};

class AppQRComponent : public juce::DocumentWindow {
public:
	AppQRComponent(ApplicationSettings& appSettings) : DocumentWindow("Scan to connect", juce::Colours::white, 5) {
		setUsingNativeTitleBar(true);
		setResizable(true, true);
		setContentOwned(new AppQRContentComponent(appSettings), true);
		centreWithSize(600, 500);
	}

	void closeButtonPressed() override {
		setVisible(false);
	}

private:

};