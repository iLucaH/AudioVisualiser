/*
  ==============================================================================

    PanelComponent.h
    Created: 2 Oct 2026 12:44:55pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <stdio.h>
#include "StrHelper.h"
#include "VideoEncoder.h"
#include "AVAPIResolver.h"
#include "Settings.h"
#include "WebViewHelper.h"

#include "EventBus.h"

class PanelComponent : public juce::Component {
public:
	PanelComponent(SelectorTabPanel& selectorTabPanel) : selectorTabPanel(selectorTabPanel), webView(createWebViewOptions()) {
		// Web View Management
		DBG("Panel Loging Searching for resource provider root.");
		webView.goToURL(webView.getResourceProviderRoot()); // Ask c++ backend for the resource.
		DBG("WebView Location set to Root: " << webView.getResourceProviderRoot());
		addAndMakeVisible(webView);

		// Event Bus Management
		for (const auto event : Send_Events::Events) {
			selectorTabPanel.getEventBus().subscribe(
				event,
				[this, event](const auto& args) {
					nativeFunctionEmitEvent(juce::String(event), args);
					return juce::var();
				}
			);
		}
	}

	void resized() override {
		webView.setBounds(getLocalBounds().expanded(4, 4)); // Make the web view fit the entire window on resize.
	}

	void paint(juce::Graphics& g) override {
		g.fillAll(juce::Colour(0xff262e36));
	}

private:

	SelectorTabPanel& selectorTabPanel;

	juce::WebBrowserComponent webView;

	void nativeFunctionReceiveHandler(const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
		if (args.isEmpty()) {
			completion("ERROR! No event was provided!");
			return;
		}
		completion(selectorTabPanel.getEventBus().emit(args[0], args));
	}

	void nativeFunctionEmitEvent(juce::String eventName, const juce::var& eventMemberVariables) {
		DBG("Emiting event to backend: " << eventName);
		webView.emitEventIfBrowserIsVisible(juce::Identifier{ eventName }, eventMemberVariables);
	}

	juce::WebBrowserComponent::Options createWebViewOptions() {
		auto options = juce::WebBrowserComponent::Options{}
			.withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
			.withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{} // Change the Webview component from Explorer to the more modern and working Edge.
				.withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory))  // May get weird permission errors if no user data folder defined.
				.withBackgroundColour(juce::Colour(0xff262e36)))
			.withResourceProvider([this](const auto& url) { return getResource(url); })
			.withNativeIntegrationEnabled();

		options = options.withNativeFunction(
			"nativeFunctionMessage",
			[this](const auto& args, auto completion) {
				nativeFunctionReceiveHandler(args, std::move(completion));
			});

		return options;
	}
};