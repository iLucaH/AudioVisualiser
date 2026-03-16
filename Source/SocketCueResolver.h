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

class SocketCueResolver {
public:
    SocketCueResolver(SelectorTabPanel& selectorTabPanel) : selectorTabPanel(selectorTabPanel) {}

    bool postCue(int cueId, int body) {
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
            return false;
        }
        return true;
    }
private:
    SelectorTabPanel& selectorTabPanel;
};