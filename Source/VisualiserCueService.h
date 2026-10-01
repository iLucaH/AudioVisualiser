#pragma once

#include <JuceHeader.h>

class SelectorTabPanel;
class OpenGLComponent;

juce::String visualiser_listAll(SelectorTabPanel& selectorTabPanel);
juce::String visualiser_listCurrent(SelectorTabPanel& selectorTabPanel);
juce::String visualiser_listNext(SelectorTabPanel& selectorTabPanel);
juce::String visualiser_listPrevious(SelectorTabPanel& selectorTabPanel);

bool visualiser_setVisualiser(SelectorTabPanel& selectorTabPanel, juce::String presetName);

juce::String visualiser_effects_listAll(OpenGLComponent& openGLComponent);
juce::String visualiser_effect_info(OpenGLComponent& openGLComponent, juce::String effectID);

bool visualiser_effects_resume_all(OpenGLComponent& openGLComponent);
bool visualiser_effects_pause_all(OpenGLComponent& openGLComponent);
bool visualiser_effects_enable_one(OpenGLComponent& openGLComponent, juce::String effectID);
bool visualiser_effects_disable_one(OpenGLComponent& openGLComponent, juce::String effectID);
bool visualiser_effects_update_effect(OpenGLComponent& openGLComponent, juce::String effectID);