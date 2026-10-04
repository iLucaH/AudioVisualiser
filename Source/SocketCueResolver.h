/*
  ==============================================================================

    SocketCueResolver.h
    Created: 16 Mar 2026 1:36:01pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include "VisualiserCueService.h"
#include "EventBus.h"

#define COMMAND_AUTH 0
#define COMMAND_METRIC 1

#define COMMAND_VISUALISER_LIST_ALL 201
#define COMMAND_VISUALISER_LIST_NEXT 202
#define COMMAND_VISUALISER_LIST_PREVIOUS 203
#define COMMAND_VISUALISER_LIST_CURRENT 204
#define COMMAND_VISUALISER_SET 205
#define COMMAND_VISUALISER_MUTE 206
#define COMMAND_VISUALISER_UNMUTE 207
#define COMMAND_VISUALISER_STATE 208

#define COMMAND_EFFECTS_CURRENT_GLOBAL_STATE 301
#define COMMAND_EFFECTS_PAUSE_ALL 302
#define COMMAND_EFFECTS_RESUME_ALL 303
#define COMMAND_EFFECTS_LIST_ALL 304
#define COMMAND_EFFECTS_DISABLE_EFFECT 305
#define COMMAND_EFFECTS_ENABLE_EFFECT 306
#define COMMAND_EFFECTS_INFO_EFFECT 307
#define COMMAND_EFFECTS_UPDATE_EFFECT 308

#define COMMAND_TRACK_LIST_ALL 401
#define COMMAND_TRACK_LIST_NEXT 402
#define COMMAND_TRACK_LIST_PREVIOUS 403
#define COMMAND_TRACK_LIST_CURRENT 404
#define COMMAND_TRACK_SET 405
#define COMMAND_TRACK_STOP 406
#define COMMAND_TRACK_START 407
#define COMMAND_TRACK_CURRENT_STATE 408

#define COMMAND_RECORD_START 501
#define COMMAND_RECORD_STOP 502
#define COMMAND_RECORD_CURRENT_STATE 503

#define SETTINGS_FFT_BAND_STATE 601
#define SETTINGS_FFT_BAND_UPDATE 602
#define SETTINGS_FULLSCREEN_STATE 603
#define SETTINGS_FULLSCREEN_ENABLE 604
#define SETTINGS_FULLSCREEN_DISABLE 605
#define SETTINGS_AUDIO_MASTER 606
#define SETTINGS_AUDIO_MASTER_SET 607

#define RESPONSE_ERR "0"
#define RESPONSE_OK "1"

class SocketCueResolver {
public:
    SocketCueResolver(SelectorTabPanel& selectorTabPanel) : selectorTabPanel(selectorTabPanel) {}

    juce::String postCue(int cueId, juce::String body) {
        switch (cueId) {
        case COMMAND_AUTH:
            return message(RESPONSE_ERR, "You are already authenticated");
        case COMMAND_METRIC:
            return message(RESPONSE_OK, "some metric");

        case COMMAND_VISUALISER_LIST_ALL:
            return message(RESPONSE_OK, visualiser_listAll(selectorTabPanel));
        case COMMAND_VISUALISER_LIST_NEXT:
            return message(RESPONSE_OK, visualiser_listNext(selectorTabPanel));
        case COMMAND_VISUALISER_LIST_PREVIOUS:
            return message(RESPONSE_OK, visualiser_listPrevious(selectorTabPanel));
        case COMMAND_VISUALISER_LIST_CURRENT:
            return message(RESPONSE_OK, visualiser_listCurrent(selectorTabPanel));
        case COMMAND_VISUALISER_SET:  
            return visualiser_setVisualiser(selectorTabPanel, body)
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully set the visualiser.")
                : message(RESPONSE_ERR, "Visualiser could not be found!");
        case COMMAND_VISUALISER_MUTE:
            return visualiser_muted_set(selectorTabPanel.getOpenGLComponent(), true) 
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully muted the visualiser!" )
                : message(RESPONSE_ERR, "Failed to mute the visualiser");
        case COMMAND_VISUALISER_UNMUTE:
            return visualiser_muted_set(selectorTabPanel.getOpenGLComponent(), false) 
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully unmuted the visualiser!")
                : message(RESPONSE_ERR, "Failed to unmute the visualiser");
        case COMMAND_VISUALISER_STATE:
            return visualiser_muted(selectorTabPanel.getOpenGLComponent()) 
                ? message(RESPONSE_OK, "Enabled")
                : message(RESPONSE_OK, "Disabled");
        case COMMAND_EFFECTS_CURRENT_GLOBAL_STATE:
            return selectorTabPanel.getOpenGLComponent().getPostProcessor()->isEnabledGlobal() 
                ? message(RESPONSE_OK, "Enabled") 
                : message(RESPONSE_OK, "Disabled");
        case COMMAND_EFFECTS_PAUSE_ALL:
            return visualiser_effects_pause_all(selectorTabPanel.getOpenGLComponent())
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully disabled all visualiser effects.")
                : message(RESPONSE_ERR, "Internal error! Please contact administration.");
        case COMMAND_EFFECTS_RESUME_ALL:
            return visualiser_effects_resume_all(selectorTabPanel.getOpenGLComponent())
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully enabled all visualiser effects.")
                : message(RESPONSE_ERR, "Internal error! Please contact administration.");
        case COMMAND_EFFECTS_LIST_ALL:
            return message(RESPONSE_OK, visualiser_effects_listAll(selectorTabPanel.getOpenGLComponent()));
        case COMMAND_EFFECTS_DISABLE_EFFECT:
            return visualiser_effects_disable_one(selectorTabPanel.getOpenGLComponent(), body)
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully disabled the visualiser effect: " + body + ".")
                : message(RESPONSE_ERR, "Failed to disable the visualiser effect: " + body + "! Effect could not be found!");
        case COMMAND_EFFECTS_ENABLE_EFFECT:
            return visualiser_effects_enable_one(selectorTabPanel.getOpenGLComponent(), body)
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully enabled the visualiser effect: " + body + ".")
                : message(RESPONSE_ERR, "Failed to enable the visualiser effect: " + body + "! Effect could not be found!");
        case COMMAND_EFFECTS_INFO_EFFECT:
            return message(RESPONSE_OK, visualiser_effect_info(selectorTabPanel.getOpenGLComponent(), body));
        case COMMAND_EFFECTS_UPDATE_EFFECT:
            return visualiser_effects_update_effect(selectorTabPanel.getOpenGLComponent(), body) 
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully updated the effect!")
                : message(RESPONSE_ERR, "Failed to update the effect!");

        case COMMAND_TRACK_LIST_ALL:
            return message(RESPONSE_OK, "Template message response.");
        case COMMAND_TRACK_LIST_NEXT:
            return message(RESPONSE_OK, "Template message response.");
        case COMMAND_TRACK_LIST_PREVIOUS:
            return message(RESPONSE_OK, "Template message response.");
        case COMMAND_TRACK_LIST_CURRENT:
            return message(RESPONSE_OK, "Template message response.");
        case COMMAND_TRACK_SET:
            return message(RESPONSE_OK, "Template message response.");
        case COMMAND_TRACK_STOP:
            selectorTabPanel.processStop();
            return messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Stopping the track.");
        case COMMAND_TRACK_START:
            selectorTabPanel.processPlay();
            return messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Starting the track.");
        case COMMAND_TRACK_CURRENT_STATE:
            return message(RESPONSE_OK, "Template message response.");

        case COMMAND_RECORD_START:
            return recording_start(selectorTabPanel.getOpenGLComponent()) 
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully started recording!")
                : message(RESPONSE_ERR, "Failed to start recording!");
        case COMMAND_RECORD_STOP:
            return recording_stop(selectorTabPanel.getOpenGLComponent()) 
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully stopped recording!")
                : message(RESPONSE_ERR, "Failed to stop recording!");
        case COMMAND_RECORD_CURRENT_STATE:
            return recording_state(selectorTabPanel.getOpenGLComponent()) 
                ? message(RESPONSE_OK, "Active") 
                : message(RESPONSE_OK, "Inactive");

        case SETTINGS_FFT_BAND_STATE:
            return message(RESPONSE_OK, settings_fft_state(selectorTabPanel));
        case SETTINGS_FFT_BAND_UPDATE:
            return settings_fft_state_set(selectorTabPanel, body) 
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully set the new state to " + body)
                : message(RESPONSE_ERR, "Failed  to update FFT band!");
        case SETTINGS_FULLSCREEN_STATE:
            return message(RESPONSE_OK, settings_visualiser_fullscreen_state(selectorTabPanel.getOpenGLComponent()) ? "True" : "False");
        case SETTINGS_FULLSCREEN_ENABLE:
            return settings_visualiser_fullscreen_set_state(selectorTabPanel.getOpenGLComponent(), true) 
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully set the visualiser to fullscreen!")
                : message(RESPONSE_ERR, "Failed to update state!");
        case SETTINGS_FULLSCREEN_DISABLE:
            return settings_visualiser_fullscreen_set_state(selectorTabPanel.getOpenGLComponent(), false) 
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Screen minimized successfully!")
                : message(RESPONSE_ERR, "Failed to update state!");
        case SETTINGS_AUDIO_MASTER:
            return message(RESPONSE_OK, juce::String(settings_audio_master_state(selectorTabPanel)));
        case SETTINGS_AUDIO_MASTER_SET:
            return settings_audio_master_state_set(selectorTabPanel, body) 
                ? messageAndThenSendUpdateToFrontend(RESPONSE_OK, "Successfully set the audio master scalar to " + body)
                : message(RESPONSE_ERR, "Failed to update master scalar!");
        default:
            return message(RESPONSE_ERR, "You have sent an unknown command!");
        }
    }

    juce::String getClientAuthPassword() {
        return selectorTabPanel.getAppSettings().getSocketClientAuth();
    }
private:
    SelectorTabPanel& selectorTabPanel;

    juce::String message(juce::String type, juce::String response) {
        return type + juce::String(" ") + response;
    }

    juce::String messageAndThenSendUpdateToFrontend(juce::String type, juce::String response) {
        juce::String s(type + juce::String(" ") + response);
        sendUpdateEventToFrontend();
        return s;
    }

    void sendUpdateEventToFrontend() {
        selectorTabPanel.getEventBus().emit(Send_Events::GlobalSocketUpdateAll);
    }
};