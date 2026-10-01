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
#include "OpenGLComponent.h"

juce::String visualiser_listAll(SelectorTabPanel& selectorTabPanel)
{
    juce::Array<juce::var> varArray;

    for (const auto* item : selectorTabPanel.getRenderProfiles())
    {
        if (item != nullptr)
            varArray.add(item->getComponentID() + juce::String(" ") + item->getPresetName());
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

bool visualiser_setVisualiser(SelectorTabPanel& selectorTabPanel, juce::String presetName) {
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

juce::String visualiser_effect_info(OpenGLComponent& openGLComponent, juce::String effectID) {
    juce::DynamicObject* obj = new juce::DynamicObject();
    obj->setProperty("id", "unknown");
    obj->setProperty("name", "unknown");
    obj->setProperty("priority", "unknown");
    obj->setProperty("enabled", false);

    juce::DynamicObject* properties = new juce::DynamicObject();
    obj->setProperty("properties", properties);

    for (auto& effect : openGLComponent.getPostProcessor()->getPostProcessEffects()) {
        if (effect == nullptr)
            continue;
        if (!juce::String(effect->getEffectID()).equalsIgnoreCase(effectID)) // compare by string to avoid error handling converting effectId to int
            continue;

        obj->setProperty("id", effect->getEffectID()); // int form for consistency 
        obj->setProperty("name", effect->getEffectName());
        obj->setProperty("priority", static_cast<int>(effect->getPriority()));
        obj->setProperty("enabled", effect->isEnabled());

        // For when properties support is added, e.g. modifying shader values, etc.
        juce::DynamicObject* properties = new juce::DynamicObject();
        obj->setProperty("properties", properties);
        
        break;
    }
    juce::var json(obj);
    return juce::JSON::toString(json);
}

juce::String visualiser_effects_listAll(OpenGLComponent& openGLComponent) {
    juce::Array<juce::var> varArray;

    for (auto& effect : openGLComponent.getPostProcessor()->getPostProcessEffects()) {
        if (effect != nullptr)
            varArray.add(juce::String("ID: ") + juce::String(effect->getEffectID()) + juce::String(" Name: ") + effect->getEffectName());
    }

    return juce::JSON::toString(juce::var(varArray), true);
}

bool visualiser_effects_resume_all(OpenGLComponent& openGLComponent) {
    openGLComponent.getPostProcessor()->setEnabled(true);
    return true;
}

bool visualiser_effects_pause_all(OpenGLComponent& openGLComponent) {
    openGLComponent.getPostProcessor()->setEnabled(false);
    return true;
}

bool visualiser_effects_enable_one(OpenGLComponent& openGLComponent, juce::String effectID) {
    for (auto& effect : openGLComponent.getPostProcessor()->getPostProcessEffects()) {
        if (effect == nullptr)
            continue;
        if (!juce::String(effect->getEffectID()).equalsIgnoreCase(effectID))
            continue;
        effect->setEnabled(true);
        return true;
    }
    return false;
}
bool visualiser_effects_disable_one(OpenGLComponent& openGLComponent, juce::String effectID) {
    for (auto& effect : openGLComponent.getPostProcessor()->getPostProcessEffects()) {
        if (effect == nullptr)
            continue;
        if (!juce::String(effect->getEffectID()).equalsIgnoreCase(effectID))
            continue;
        effect->setEnabled(false);
        return true;
    }
    return false;
}

bool visualiser_effects_update_effect(OpenGLComponent& openGLComponent, juce::String effectID) {

}