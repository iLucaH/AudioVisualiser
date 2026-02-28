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
#include "VideoEncoder.h"

#define UPLOAD_NO_STATE 0
#define UPLOAD_SUBMITTED 1
#define UPLOAD_CONFIRMED 2
#define UPLOAD_FAILED 3

// 600 x 300
class ContentComponent : public juce::Component, private juce::Timer {
public:
	ContentComponent(OpenGLComponent& openGLComponent) : glComponent(openGLComponent) {
		addAndMakeVisible(startButton);
		addAndMakeVisible(finishButton);
		addAndMakeVisible(fileNameEditor);
		addAndMakeVisible(pathNameButton);

		// Youtube UI logic
		addAndMakeVisible(uploadVideoButton);
		addAndMakeVisible(videoTitleEditor);
		addAndMakeVisible(videoDescriptionEditor);
		addAndMakeVisible(isPublicButton);
		addAndMakeVisible(submitButton);
		addAndMakeVisible(cancelButton);
		showYoutubeButtons(false); // lets now hide the video buttons because we know that there is no video yet to be uploaded.

		startButton.setBounds(50, 20, 100, 30);
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

			auto* str = new juce::String(outputFile.getFullPathName());
			glComponent.pendingEncoderFileName.store(str);

			recordingStartTime = juce::Time::getMillisecondCounter();
			startTimerHz(1);

			state = true;

			// Everytime a recording starts, we know that there is no video to be uploaded to youtube, so this is a good time to
			// hide the youtube buttons until the video ends in which we can bring the buttons back up.
			showYoutubeButtons(false);
			repaint();

		};

		finishButton.setBounds(450, 20, 100, 30);
		finishButton.onClick = [this] {
			if (!state) // if we aren't already running, then we won't need to do any of this logic.
				return;
			glComponent.pendingStop.store(true);

			stopTimer();
			elapsedTimeString = "00:00";

			state = false;

			// Now that the video has finished, we can prompt the user to upload the video to youtube.
			uploadVideoButton.setVisible(true);
			repaint();
		};

		pathNameButton.setBounds(200, 120, 200, 30);
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

		fileNameEditor.setBounds(200, 70, 200, 30);
		fileNameEditor.onReturnKey = [this] {
			// validate input
			if (isValidVidFileStr(fileNameEditor.getText().toStdString())) {
				fileNameFound = true;
				fileNameEditor.setReadOnly(true);
			} else {
				fileNameEditor.setText(juce::String("Please enter a valid file name!"));
			}
		};

		uploadVideoButton.setBounds(50, 170, 100, 55);
		uploadVideoButton.onClick = [this] {
			showYoutubeButtons(true);
			uploadVideoButton.setVisible(true); // this button is the exception for this use case.
			};

		submitButton.setBounds(450, 190, 100, 30);
		submitButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightseagreen);
		submitButton.onClick = [this] {
			uploadingState.store(UPLOAD_SUBMITTED);
			repaint();
				// submit logic. Send on new thread with atomic current update string callbacks to this thread
			};

		cancelButton.setBounds(450, 240, 100, 30);
		cancelButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::palevioletred);
		cancelButton.onClick = [this] {
			showYoutubeButtons(false);
			uploadVideoButton.setVisible(true);
			uploadingState.store(UPLOAD_NO_STATE);
			};

		isPublicButton.setBounds(125, 232, 50, 50);
		isPublicButton.setState(juce::ToggleButton::buttonOver);

		videoTitleEditor.setBounds(200, 170, 200, 30);
		videoTitleEditor.setText("My Video Title!");

		videoDescriptionEditor.setBounds(200, 205, 200, 90);
		videoDescriptionEditor.setText("My Video Description!");
		videoDescriptionEditor.setMultiLine(true, true);
		videoDescriptionEditor.setReturnKeyStartsNewLine(true);
		videoDescriptionEditor.setScrollbarsShown(true);
	}

	void paint(juce::Graphics& g) override {
		g.fillAll(state ? juce::Colours::green : juce::Colours::red);
		g.drawSingleLineText("File Name:", 160, 90, juce::Justification(0));
		g.drawSingleLineText("Output Path:", 160, 140, juce::Justification(0));
		g.drawSingleLineText(elapsedTimeString, 310, 42, juce::Justification(0));
		if (isPublicButton.isVisible()) // Only draw if the button it is describing is actually visible
			g.drawSingleLineText("Private:", 100, 260, juce::Justification(0));
		if (uploadingState.load() != UPLOAD_NO_STATE) // If there is an uploading state that is not the idle state, then we should display it as a message.
			g.drawSingleLineText("Status: " + getYTUploadStatusMessage(uploadingState.load()), 560, 236, juce::Justification(0));
	}

	void timerCallback() override {
		// We need to know if at any point the timer should be stopped because the video recording 
		// was cancelled due to other reasons. Also, if the timer has reached above the maximum
		// defined allowable recording time limit, then we should force a cancel now in the timer loop.
		bool recordingShouldContinue = glComponent.getVideoEncoder()->isActive();

		auto now = juce::Time::getMillisecondCounter();
		auto elapsedMs = now - recordingStartTime;

		if (elapsedMs < MAX_RECORDING_LENGTH && recordingShouldContinue) {
			// Keep recording because we have not passed the time limit.
			int totalSeconds = static_cast<int>(elapsedMs / 1000);
			int minutes = totalSeconds / 60;
			int seconds = totalSeconds % 60;
			elapsedTimeString = juce::String(minutes).paddedLeft('0', 2) + ":" + juce::String(seconds).paddedLeft('0', 2);
			repaint();
		} else {
			// Time limit has passed so lets can the recording. Lets also stop if the recording is set to stopped already elsewhere.
			finishButton.onClick();
		}
	}

	void showYoutubeButtons(bool show) {
		uploadVideoButton.setVisible(show);
		videoTitleEditor.setVisible(show);
		videoDescriptionEditor.setVisible(show);
		isPublicButton.setVisible(show);
		submitButton.setVisible(show);
		cancelButton.setVisible(show);
		repaint();
	}

private:
	OpenGLComponent& glComponent;

	bool state = false, filePathFound = false, fileNameFound = false;
	std::atomic<int> uploadingState{ 0 }; // The status of trying to upload to youtube.
	
	// Recording Buttons
	juce::TextButton startButton{ "Start" };
	juce::TextButton finishButton{ "Finish" };

	// Path Buttons
	juce::FileChooser pathSelector{ "Please select the directory you want to export to...", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*.mp4" };
	juce::TextEditor fileNameEditor{ "TypeFileName" };
	juce::TextButton pathNameButton{ "No Path Selected!" };

	juce::int64 recordingStartTime = 0;
	juce::String elapsedTimeString = "00:00";

	// Video Upload Buttons
	juce::TextButton uploadVideoButton{ "Upload Video to Youtube" };
	juce::TextEditor videoTitleEditor{ "VideoTitle" };
	juce::TextEditor videoDescriptionEditor{ "VideoDescription!" };
	juce::ToggleButton isPublicButton;
	juce::TextButton submitButton{ "Confirm Upload" };
	juce::TextButton cancelButton{ "Cancel Upload" };

	juce::String getYTUploadStatusMessage(int youtubeUploadStatus) {
		if (youtubeUploadStatus == UPLOAD_FAILED) {
			return "FAILED";
		} else if (youtubeUploadStatus == UPLOAD_SUBMITTED) {
			return "SUBMITTED";
		} else if (youtubeUploadStatus == UPLOAD_CONFIRMED) {
			return "CONFIRMED";
		} else {
			return "";
		}
	}

};

class CreateVideoComponent : public juce::DocumentWindow {
public:
	CreateVideoComponent(OpenGLComponent& openGLComponent) : DocumentWindow("recorder!", juce::Colours::white, 5), openGLComponent(openGLComponent) {
		setUsingNativeTitleBar(true);

		setContentOwned(new ContentComponent(openGLComponent), true);
	}

	void closeButtonPressed() override {
		setVisible(false);
	}

private:
	OpenGLComponent& openGLComponent;

};