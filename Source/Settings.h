/*
  ==============================================================================

    Settings.h
    Created: 3 Mar 2026 3:46:50pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class AudioVisualiserAudioProcessorEditor;

class ApplicationSettings {
public:
    ApplicationSettings(AudioVisualiserAudioProcessorEditor* editor);
    /*
        Only to be updated on the message thread for now.
    */
    void setAuthJWT(juce::String jwt) {
        authJWT = jwt;
    }

    bool isAuth() const {
        return authJWT != "";
    }

    juce::String getAuthJWT() const {
        return authJWT;
    }

    void setDimensions(int w, int h) {
        width = w;
        height = h;
        sendDimensionUpdate(w, h);
    }

    int getWidth() {
        return width;
    }

    int getHeight() {
        return height;
    }

    int getFFTSize() {
        return fftSize;
    }

    void setFFTSize(int size) {
        fftSize = size;
    }

private:
    AudioVisualiserAudioProcessorEditor* root;

    void sendDimensionUpdate(int w, int h);

    juce::String authJWT = "";

    int width = 1920, height = 1080;
    int fftSize = 2048;
    bool fullScreen = false;
};