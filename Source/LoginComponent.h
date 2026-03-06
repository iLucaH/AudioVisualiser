/*
  ==============================================================================

    LoginComponent.h
    Created: 28 Feb 2026 10:37:55am
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

#define UPLOAD_NO_STATE 0
#define UPLOAD_SUBMITTED 1
#define UPLOAD_CONFIRMED 2
#define UPLOAD_FAILED 3

#define LOGIN_ATTEMPT_ARGS_OK 10
#define LOGIN_ATTEMPT_NOT_ENOUGH_ARGS 11
#define LOGIN_ATTEMPT_INVALID_USERNAME 12
#define LOGIN_ATTEMPT_INVALID_PASSWORD 13

// 600 x 300
class LoginContentComponent : public juce::Component, private juce::Timer {
public:
	LoginContentComponent(ApplicationSettings& appSettings) : 
		settings(appSettings), 
		webView{ juce::WebBrowserComponent::Options{}
			.withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
			.withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{} // Change the Webview component from Explorer to the more modern and working Edge.
				.withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory))  // May get weird permission errors if no user data folder defined.
				.withBackgroundColour(juce::Colours::white))
			.withResourceProvider([this](const auto& url) { return getResource(url); })
			.withNativeIntegrationEnabled()
			.withNativeFunction(juce::Identifier{"nativeFunctionLogin"}, [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
				nativeFunctionLogin(args, std::move(completion));
				})} {

		webView.goToURL(webView.getResourceProviderRoot()); // Ask c++ backend for the resource.
		DBG("WebView Location set to Root: " << webView.getResourceProviderRoot());

		addAndMakeVisible(webView);
	}

	void resized() override {
		webView.setBounds(getLocalBounds()); // Make the web view fit the entire window on resize.
	}

	void timerCallback() override {
	}

private:

	ApplicationSettings& settings;

	juce::WebBrowserComponent webView;

	std::atomic<bool> isAttemptingLogin{ false };

	//juce::TextEditor username{ "Username" };
	//juce::TextEditor password{ "Passowrd" };
	//juce::TextButton login{ "Login" };

	void nativeFunctionLogin(const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
		bool currentlyLoggingIn = isAttemptingLogin.load();
		if (currentlyLoggingIn) // If there is already a login attempt present, no need to continue with it.
			return;
		if (args.size() != 2) {
			completion(LOGIN_ATTEMPT_NOT_ENOUGH_ARGS);
			return;
		}
		juce::String username = args[0];
		if (username.length() <= 0) {
			completion(LOGIN_ATTEMPT_INVALID_USERNAME);
			return;
		}
		juce::String password = args[1];
		if (password.length() <= 0) {
			completion(LOGIN_ATTEMPT_INVALID_PASSWORD);
			return;
		}
		juce::String concatenatedArgs;
		for (const auto& arg : args) {
			concatenatedArgs += arg.toString();
		}
		DBG("Native Login Function called from front end to back end. Args: " << concatenatedArgs);
		completion(LOGIN_ATTEMPT_ARGS_OK);
		isAttemptingLogin.store(true);
		juce::String token = api_login(username, password);
		isAttemptingLogin.store(false);
		// Validate token here.
		if (token.length() == 0) {
			// Failed to validate login credentials.
			DBG("Failed to validate api login JWT!");
			webView.emitEventIfBrowserIsVisible(juce::Identifier{ "onLoginEvent" }, false); // Example function for later implementation
			return;
		}
		// Successfully validated login credentials.
		settings.setAuthJWT(token);
		DBG("Successfully validated api login JWT!");
		webView.emitEventIfBrowserIsVisible(juce::Identifier{ "onLoginEvent" }, true); // Example function for later implementation
	}

};

class LoginComponent : public juce::DocumentWindow {
public:
	LoginComponent(ApplicationSettings& appSettings) : DocumentWindow("Accounts and Login", juce::Colours::white, 5) {
		setUsingNativeTitleBar(true);

		setContentOwned(new LoginContentComponent(appSettings), true);
	}

	void closeButtonPressed() override {
		setVisible(false);
	}

private:

};