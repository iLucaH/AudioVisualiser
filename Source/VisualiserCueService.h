#pragma once

#include <JuceHeader.h>

class SelectorTabPanel;

juce::String visualiser_listAll(SelectorTabPanel& selectorTabPanel);
juce::String visualiser_listCurrent(SelectorTabPanel& selectorTabPanel);
juce::String visualiser_listNext(SelectorTabPanel& selectorTabPanel);
juce::String visualiser_listPrevious(SelectorTabPanel& selectorTabPanel);

bool visualiser_setVisualiser(SelectorTabPanel& selectorTabPanel, juce::String presetName);