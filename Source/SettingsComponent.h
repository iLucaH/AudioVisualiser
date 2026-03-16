/*
  ==============================================================================

    SettingsComponent.h
    Created: 13 Mar 2026 12:00:11pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Settings.h"
#include "WebViewHelper.h"

#define SETTINGS_DIMENSION_W 0
#define SETTINGS_DIMENSION_H 1
#define SETTINGS_DIMENSION_WH 2
#define SETTINGS_FFT_SIZE 3

#define MIN_WIDTH 100
#define MAX_WIDTH 1920
#define MIN_HEIGHT 100
#define MAX_HEIGHT 1080

class SettingsContentComponent : public juce::Component{
public:
	SettingsContentComponent(ApplicationSettings& appSettings) : 
		settings(appSettings),
		webView{ juce::WebBrowserComponent::Options{}
			.withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
			.withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
				.withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory))
				.withBackgroundColour(juce::Colours::white))
			.withResourceProvider([this](const auto& url) { return getResource(url); })
			.withNativeFunction(juce::Identifier{"nativeFunctionChangeSettings"}, [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
				nativeFunctionChangeSettings(args, std::move(completion));
				})
			.withNativeFunction(juce::Identifier{"nativeFunctionGetSettings"}, [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
				nativeFunctionGetSettings(args, std::move(completion));
				})
			.withNativeIntegrationEnabled()} {

		webView.goToURL(webView.getResourceProviderRoot() + "settings.html");
		DBG("WebView Location set to Root: " << webView.getResourceProviderRoot());

		addAndMakeVisible(webView);
	}

	void resized() override {
		webView.setBounds(getLocalBounds()); // Make the web view fit the entire window on resize.
	}

private:
	ApplicationSettings& settings;
	juce::WebBrowserComponent webView;

	void nativeFunctionGetSettings(const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
		if (args.size() < 1) {
			completion(-1);
			return;
		}
		int setting = args[0];
		switch (setting) {
			case SETTINGS_DIMENSION_W:
				completion(settings.getWidth());
				break;
			case SETTINGS_DIMENSION_H:
				completion(settings.getHeight());
				break;
			case SETTINGS_FFT_SIZE:
				completion(settings.getFFTSize());
				break;
			default:
				completion(-1);
			}
	}

	void nativeFunctionChangeSettings(const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
		if (args.size() < 2) {
			DBG("Settings change attempted but args size is too small!");
			completion(false);
			return;
		}
		int setting = args[0].isInt() ? (int) args[0] : -1;
		int fftSize;

		switch (setting) {
		case SETTINGS_DIMENSION_WH:
			if (args.size() >= 3) {
				int width = std::stoi(args[1].toString().toStdString());
				int height = std::stoi(args[2].toString().toStdString());
				if (width < MIN_WIDTH || width > MAX_WIDTH || height < MIN_HEIGHT || height > MAX_HEIGHT) {
					DBG("WH Settings attempted to change but the width and height are outside the acceptable bounds!");
					DBG("Bounds: w=" << args[1].toString() << " h=" << args[2].toString() << " with casts w=" << width << " h=" << height);
					completion(false);
					break;
				}
				settings.setDimensions(width, height);
				completion(true);
				break;
			}
			completion(false);
			break;
		case SETTINGS_FFT_SIZE:
			fftSize = args[1];
			settings.setFFTSize(fftSize);
			completion(true);
			break;
		default:
			DBG("Settings change attempted but the settigns ID was unkown! Setting: " << args[0].toString());
			completion(false);
		}
	}
};

class SettingsComponent : public juce::DocumentWindow {
public:
	SettingsComponent(ApplicationSettings& appSettings) : DocumentWindow("Settings", juce::Colours::white, 5) {
		setUsingNativeTitleBar(true);
		setResizable(true, true);
		setContentOwned(new SettingsContentComponent(appSettings), true);
		centreWithSize(600, 600);
	}

	void closeButtonPressed() override {
		setVisible(false);
	}

private:

};