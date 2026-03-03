/*
  ==============================================================================

    Settings.h
    Created: 3 Mar 2026 3:46:50pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ApplicationSettings {
public:
    ApplicationSettings() {

    }

    void setAuthJWT(juce::String jwt) {
        authJWT = jwt;
    }

private:
    juce::String authJWT = "";
};