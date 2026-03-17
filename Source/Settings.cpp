/*
  ==============================================================================

    Settings.cpp
    Created: 16 Mar 2026 2:49:18pm
    Author:  lucas

  ==============================================================================
*/

#include "Settings.h"
#include "PluginEditor.h"

ApplicationSettings::ApplicationSettings(AudioVisualiserAudioProcessorEditor* editor) : root(editor) {

}

void ApplicationSettings::sendDimensionUpdate(int w, int h) {
    root->getOpenGLComponent().resetVideoRecorder(w, h);
}

void ApplicationSettings::setFullScreen(bool val) {
    root->getOpenGLComponent().setFullScreen(val);
    if (val == false) {
        root->setSize(1080, 544);
        root->centreWithSize(1080, 544);
    }
}