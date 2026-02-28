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

#define UPLOAD_NO_STATE 0
#define UPLOAD_SUBMITTED 1
#define UPLOAD_CONFIRMED 2
#define UPLOAD_FAILED 3

// 600 x 300
class LoginContentComponent : public juce::Component, private juce::Timer {
public:
	LoginContentComponent() {
	}

	void paint(juce::Graphics& g) override {
	}

	void timerCallback() override {
	}

};

class LoginComponent : public juce::DocumentWindow {
public:
	LoginComponent() : DocumentWindow("Accounts and Login", juce::Colours::white, 5) {
		setUsingNativeTitleBar(true);

		setContentOwned(new LoginContentComponent(), true);
	}

	void closeButtonPressed() override {
		setVisible(false);
	}

private:

};