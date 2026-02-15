/*
  ==============================================================================

    CreateVideoComponent.h
    Created: 15 Feb 2026 2:27:45pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <stdio.h>

// 600 x 300
class ContentComponent : public juce::Component {
public:
    ContentComponent() {
        addAndMakeVisible(startButton);
        addAndMakeVisible(finishButton);

        startButton.setBounds(50, 50, 100, 30);
        finishButton.setBounds(450, 50, 100, 30);

        startButton.onClick = [this] { state = true; repaint(); };
        finishButton.onClick = [this] { state = false; repaint(); };

        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        pathSelector.launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser) {
                juce::File mooseFile(chooser.getResult());

                filePathFound = true;
                repaint();
            });
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(state && filePathFound ? juce::Colours::green
            : juce::Colours::red);
    }

private:
    bool state = false, filePathFound = false;
    juce::TextButton startButton{ "Start" };
    juce::TextButton finishButton{ "Finish" };
    juce::FileChooser pathSelector{ "Please select the directory you want to export to...", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*.mp4" };
};

class CreateVideoComponent : public juce::DocumentWindow {
public:
    CreateVideoComponent() : DocumentWindow("recorder!", juce::Colours::white, 5)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, true);

        setContentOwned(new ContentComponent(), true);

        centreWithSize(300, 200);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        setVisible(false);
    }
};