#pragma once

#include <JuceHeader.h>

class SelectorTabPanel;
class OpenGLComponent;

juce::String visualiser_listAll(SelectorTabPanel& selectorTabPanel);
juce::String visualiser_listCurrent(SelectorTabPanel& selectorTabPanel);
juce::String visualiser_listNext(SelectorTabPanel& selectorTabPanel);
juce::String visualiser_listPrevious(SelectorTabPanel& selectorTabPanel);

bool visualiser_setVisualiser(SelectorTabPanel& selectorTabPanel, juce::String presetName);

bool visualiser_muted(OpenGLComponent& openGLComponent);
bool visualiser_muted_set(OpenGLComponent& openGLComponent, bool muted);

juce::String visualiser_effects_listAll(OpenGLComponent& openGLComponent);
juce::String visualiser_effect_info(OpenGLComponent& openGLComponent, juce::String effectID);

bool visualiser_effects_resume_all(OpenGLComponent& openGLComponent);
bool visualiser_effects_pause_all(OpenGLComponent& openGLComponent);
bool visualiser_effects_enable_one(OpenGLComponent& openGLComponent, juce::String effectID);
bool visualiser_effects_disable_one(OpenGLComponent& openGLComponent, juce::String effectID);
bool visualiser_effects_update_effect(OpenGLComponent& openGLComponent, juce::String effectID);

bool recording_start(OpenGLComponent& openGLComponent);
bool recording_stop(OpenGLComponent& openGLComponent);
bool recording_state(OpenGLComponent& openGLComponent);

juce::String settings_fft_state(SelectorTabPanel& selectorTabPanel);
bool settings_fft_state_set(SelectorTabPanel& selectorTabPanel, juce::String band);

bool settings_visualiser_fullscreen_state(OpenGLComponent& openGLComponent);
bool settings_visualiser_fullscreen_set_state(OpenGLComponent& openGLComponent, juce::String state);

float settings_audio_master_state(SelectorTabPanel& selectorTabPanel);
bool settings_audio_master_state_set(SelectorTabPanel& selectorTabPanel, juce::String scalar);