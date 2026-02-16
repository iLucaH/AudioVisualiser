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
#include "StrHelper.h"

// 600 x 300
class ContentComponent : public juce::Component {
public:
	ContentComponent(OpenGLComponent& openGLComponent) : glComponent(openGLComponent) {
		addAndMakeVisible(startButton);
		addAndMakeVisible(finishButton);
		addAndMakeVisible(fileNameEditor);
		addAndMakeVisible(pathNameButton);

		startButton.setBounds(50, 50, 100, 30);
		finishButton.setBounds(450, 50, 100, 30);
		fileNameEditor.setBounds(200, 100, 200, 30);
		pathNameButton.setBounds(200, 150, 200, 30);

		startButton.onClick = [this] {
			// check if video encoder is null because it is initialised as a unique pointer at time of gl context creation.
			// if null, it has not yet been initialised and we cannot proceed.
			if (!glComponent.getVideoEncoder()) {
				return;
			}
			if (!filePathFound || !fileNameFound)
				return;
			juce::File outputFile(pathNameButton.getButtonText());
			outputFile = outputFile.getChildFile(fileNameEditor.getText());
			glComponent.getVideoEncoder()->setFileName(outputFile.getFullPathName());
			if (glComponent.getVideoEncoder()->startRecordingSession()) {
				repaint();
				state = true;
			} else {
				DBG("Failed to start recording session!");
				// handle error.
			}
		};
		finishButton.onClick = [this] {
			if (!state) // if we aren't already running, then we won't need to do any of this logic.
				return;
			if (glComponent.getVideoEncoder()->finishRecordingSession()) {
				repaint();
			} else {
				DBG("Failed to end recording session!");
				// handle error.
			}
			state = false; // state should be false even if finishing the session fails.
		};

		pathNameButton.onClick = [this] {
			if (filePathFound) {
				// Selecting multiple times isn't supported yet because that would be a race condition for filePathFound;
				return;
			}
			auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
			pathSelector.launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser) {
				juce::File outputFile(chooser.getResult());
				if (!outputFile.getFullPathName().isEmpty()) {
					filePathFound = true;
					pathNameButton.setButtonText(outputFile.getFullPathName());
					repaint();
				}
			});
		};
		fileNameEditor.onReturnKey = [this] {
			// validate input
			if (isValidVidFileStr(fileNameEditor.getText().toStdString())) {
				fileNameFound = true;
				fileNameEditor.setReadOnly(true);
			} else {
				fileNameEditor.setText(juce::String("Please enter a valid file name!"));
			}
		};
	}

	void paint(juce::Graphics& g) override {
		g.fillAll(state ? juce::Colours::green : juce::Colours::red);
		g.drawSingleLineText("File Name:", 160, 120, juce::Justification(0));
		g.drawSingleLineText("Output Path:", 160, 170, juce::Justification(0));
		g.drawSingleLineText("00:00", 310, 72, juce::Justification(0));
	}

private:
	OpenGLComponent& glComponent;

	bool state = false, filePathFound = false, fileNameFound = false;
	juce::TextButton startButton{ "Start" };
	juce::TextButton finishButton{ "Finish" };
	juce::FileChooser pathSelector{ "Please select the directory you want to export to...", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*.mp4" };
	juce::TextEditor fileNameEditor{ "TypeFileName" };
	juce::TextButton pathNameButton{ "No Path Selected!" };
};

class CreateVideoComponent : public juce::DocumentWindow {
public:
	CreateVideoComponent(OpenGLComponent& openGLComponent) : DocumentWindow("recorder!", juce::Colours::white, 5), openGLComponent(openGLComponent) {
		setUsingNativeTitleBar(true);
		setResizable(true, true);

		setContentOwned(new ContentComponent(openGLComponent), true);
	}

	void closeButtonPressed() override {
		setVisible(false);
	}

private:
	OpenGLComponent& openGLComponent;

};