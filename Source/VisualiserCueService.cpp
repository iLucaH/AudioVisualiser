/*
  ==============================================================================

    VisualiserCueService.cpp
    Created: 4 Sep 2026 12:24:24am
    Author:  lucas

  ==============================================================================
*/

#include <JuceHeader.h>

#include "VisualiserCueService.h"
#include "SelectorTabPanel.h"

juce::String visualiser_listAll(SelectorTabPanel& selectorTabPanel)
{
    juce::Array<juce::var> varArray;

    for (const auto* item : selectorTabPanel.getRenderProfiles())
    {
        if (item != nullptr)
            varArray.add(item->getPresetName());
    }

    return juce::JSON::toString(juce::var(varArray), true);
}

juce::String visualiser_listCurrent(SelectorTabPanel& selectorTabPanel)
{
    const auto& renderProfiles = selectorTabPanel.getRenderProfiles();

    if (renderProfiles.empty())
        return {};

    const int currentIndex =
        static_cast<int>(selectorTabPanel.getCurrentState()) - 1;

    if (currentIndex < 0 ||
        currentIndex >= static_cast<int>(renderProfiles.size()))
    {
        return {};
    }

    if (renderProfiles[currentIndex] == nullptr)
        return {};

    return renderProfiles[currentIndex]->getPresetName();
}

juce::String visualiser_listNext(SelectorTabPanel& selectorTabPanel)
{
    const auto& renderProfiles = selectorTabPanel.getRenderProfiles();

    if (renderProfiles.empty())
        return {};

    const int currentIndex =
        static_cast<int>(selectorTabPanel.getCurrentState()) - 1;

    if (currentIndex < 0 ||
        currentIndex >= static_cast<int>(renderProfiles.size()))
    {
        return {};
    }

    const int nextIndex =
        (currentIndex + 1) % static_cast<int>(renderProfiles.size());

    if (renderProfiles[nextIndex] == nullptr)
        return {};

    return renderProfiles[nextIndex]->getPresetName();
}

juce::String visualiser_listPrevious(SelectorTabPanel& selectorTabPanel)
{
    const auto& renderProfiles = selectorTabPanel.getRenderProfiles();

    if (renderProfiles.empty())
        return {};

    const int currentIndex =
        static_cast<int>(selectorTabPanel.getCurrentState()) - 1;

    if (currentIndex < 0 ||
        currentIndex >= static_cast<int>(renderProfiles.size()))
    {
        return {};
    }

    const int previousIndex =
        (currentIndex - 1 + static_cast<int>(renderProfiles.size()))
        % static_cast<int>(renderProfiles.size());

    if (renderProfiles[previousIndex] == nullptr)
        return {};

    return renderProfiles[previousIndex]->getPresetName();
}

bool visualiser_setVisualiser(
    SelectorTabPanel& selectorTabPanel,
    juce::String presetName)
{
    const auto& renderProfiles = selectorTabPanel.getRenderProfiles();

    for (int i = 0; i < static_cast<int>(renderProfiles.size()); ++i)
    {
        auto* item = renderProfiles[i];

        if (item != nullptr &&
            item->getPresetName() == presetName)
        {
            const int newState = i + 1;

            selectorTabPanel.updatePanelRenderProfile(
                newState,
                selectorTabPanel.getCurrentState()
            );

            return true;
        }
    }

    return false;
}