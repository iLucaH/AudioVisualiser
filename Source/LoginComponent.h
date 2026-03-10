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

#define REGISTER_ATTEMPT_ARGS_OK 20
#define REGISTER_ATTEMPT_NOT_ENOUGH_ARGS 21
#define REGISTER_ATTEMPT_INVALID_USERNAME 22
#define REGISTER_ATTEMPT_INVALID_PASSWORD 23

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
				})
			.withNativeFunction(juce::Identifier{"nativeFunctionRegister"}, [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
				nativeFunctionRegister(args, std::move(completion));
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
	std::atomic<bool> isAttemptingRegister{ false };


	void nativeFunctionLogin(const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
		if (isAttemptingLogin.load()) { // If there is already a login attempt present, no need to continue with it.
			// If there is already a login attempt, we can assume that there has already been args successfully passed through.
			DBG("A login attempt has been called while another login attempt was already in session!");
			completion(LOGIN_ATTEMPT_ARGS_OK);
			return;
		}
		if (args.size() != 2) { // We can ignore any args greater than two, although with a promise there shouldn't be any.
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
		DBG("Native Login Function called from front end to back end. Username: " << args[0].toString() << ", Password: " << args[1].toString() << ".");
		juce::Thread::launch([this, username, password]() {
			isAttemptingLogin.store(true);
			juce::String token = api_login(username, password);
			// Validate token here.
			bool success = token.length() > 0; // A successful token get would have substance to the string.
			if (!success) {
				DBG("Failed to validate api login JWT!");
			} else {
				settings.setAuthJWT(token);
				DBG("Successfully validated api login JWT! Token: " << token);
			}
			juce::MessageManager::callAsync([this, success]() {
				isAttemptingLogin.store(false);
				webView.emitEventIfBrowserIsVisible(juce::Identifier{ "onLoginEvent" }, success);
				}); // Post the login update back to the GUI thread.
			});
		completion(LOGIN_ATTEMPT_ARGS_OK); // Completion is sent back to the javascript frontend to have the result evaluated.
	}

	void nativeFunctionRegister(const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
		if (isAttemptingRegister.load()) {
			DBG("A register attempt has been called while another register attempt was already in session!");
			completion(REGISTER_ATTEMPT_ARGS_OK);
			return;
		}
		if (args.size() != 2) {
			completion(REGISTER_ATTEMPT_NOT_ENOUGH_ARGS);
			return;
		}
		juce::String username = args[0];
		if (username.length() <= 0) {
			completion(REGISTER_ATTEMPT_INVALID_USERNAME);
			return;
		}
		juce::String password = args[1];
		if (password.length() <= 0) {
			completion(REGISTER_ATTEMPT_INVALID_PASSWORD);
			return;
		}
		DBG("Native Register Function called from front end to back end. Username: " << args[0].toString() << ", Password: " << args[1].toString() << ".");
		juce::Thread::launch([this, username, password]() {
			isAttemptingRegister.store(true);
			int register_status = api_register(username, password);
			juce::MessageManager::callAsync([this, register_status]() {
				isAttemptingRegister.store(false);
				webView.emitEventIfBrowserIsVisible(juce::Identifier{ "onRegisterEvent" }, register_status);
				});
			});
		completion(REGISTER_ATTEMPT_ARGS_OK);
	}
};

class LoginComponent : public juce::DocumentWindow {
public:
	LoginComponent(ApplicationSettings& appSettings) : DocumentWindow("Accounts and Login", juce::Colours::white, 5) {
		setUsingNativeTitleBar(true);
		setResizable(true, true);
		setContentOwned(new LoginContentComponent(appSettings), true);
		centreWithSize(600, 600);
	}

	void closeButtonPressed() override {
		setVisible(false);
	}

private:

};