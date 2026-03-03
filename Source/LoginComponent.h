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

#define UPLOAD_NO_STATE 0
#define UPLOAD_SUBMITTED 1
#define UPLOAD_CONFIRMED 2
#define UPLOAD_FAILED 3

// 600 x 300
class LoginContentComponent : public juce::Component, private juce::Timer {
public:
	LoginContentComponent(ApplicationSettings& appSettings) : settings(appSettings) {
		username.setBounds(0, 0, 200, 25);
		username.setText("Username");
		addAndMakeVisible(username);


		password.setBounds(0, 100, 200, 25);
		password.setText("Password");
		addAndMakeVisible(password);

		login.setBounds(0, 200, 355, 355);
		login.onClick = [this] {
			juce::String token = api_login(username.getText(), password.getText());
			// Validate token here.
			if (token.length() == 0) {
				login.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::indianred);
				DBG("Failed to validate api login JWT!");
				return;
			}
			login.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lawngreen);
			settings.setAuthJWT(token);
			};
		addAndMakeVisible(login);
	}

	void paint(juce::Graphics& g) override {
	}

	void timerCallback() override {
	}

private:

	ApplicationSettings& settings;

	juce::TextEditor username{ "Username" };
	juce::TextEditor password{ "Passowrd" };
	juce::TextButton login{ "Login" };

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