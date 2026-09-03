/*
  ==============================================================================

    SocketCueResolver.h
    Created: 16 Mar 2026 1:36:01pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#define SOCKET_CUE_PLAY 0
#define SOCKET_CUE_STOP 1
#define SOCKET_CUE_RENDER_STATE_INCREMENT 2
#define SOCKET_CUE_RENDER_STATE_DECREMENT 3

#define COMMAND_AUTH 0
#define COMMAND_METRIC 1

#define COMMAND_VISUALISER_LIST_ALL 200
#define COMMAND_VISUALISER_LIST_NEXT 201
#define COMMAND_VISUALISER_LIST_PREVIOUS 202
#define COMMAND_VISUALISER_LIST_CURRENT 203
#define COMMAND_VISUALISER_SET 204
#define COMMAND_VISUALISER_MUTE 205
#define COMMAND_VISUALISER_UNMUTE 206

#define COMMAND_EFFECTS_DISABLE_ALL 301
#define COMMAND_EFFECTS_ENABLE_ALL 302
#define COMMAND_EFFECTS_LIST_ALL 303
#define COMMAND_EFFECTS_DISABLE_EFFECT 304
#define COMMAND_EFFECTS_ENABLE_EFFECT 305
#define COMMAND_EFFECTS_UPDATE_EFFECT 306

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

#define RESPONSE_ERR "0"
#define RESPONSE_OK "1"

class SocketCueResolver {
public:
    SocketCueResolver(SelectorTabPanel& selectorTabPanel) : selectorTabPanel(selectorTabPanel) {}

    juce::String postCue(int cueId, juce::String body) {
        switch (cueId) {
        case SOCKET_CUE_PLAY:
            selectorTabPanel.processPlay();
            break;
        case SOCKET_CUE_STOP:
            selectorTabPanel.processStop();
            break;
        case SOCKET_CUE_RENDER_STATE_INCREMENT:
            selectorTabPanel.processRenderStateIncrement();
            break;
        case SOCKET_CUE_RENDER_STATE_DECREMENT:
            selectorTabPanel.processRenderStateDecrement();
            break;
        default:
            return RESPONSE_ERR;
        }
        return RESPONSE_OK;
    }

    juce::String getClientAuthPassword() {
        return selectorTabPanel.getAppSettings().getSocketClientAuth();
    }
private:
    SelectorTabPanel& selectorTabPanel;
};