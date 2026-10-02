/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Settings.h"

namespace Theme
{
    const juce::Colour windowBg{ 0xff1b2128 };
    const juce::Colour titleBar{ 0xff161b21 };
    const juce::Colour card{ 0xff262e36 };
    const juce::Colour cardEdge{ 0xff5a6b7a };
    const juce::Colour text{ 0xffe6ebf0 };
    const juce::Colour textDim{ 0xff9aa6b2 };
    const juce::Colour accent{ 0xff3fd0ff };
    const juce::Colour accent2{ 0xff9b5cff };
}

//==============================================================================
AudioVisualiserAudioProcessorEditor::AudioVisualiserAudioProcessorEditor(AudioVisualiserAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), appSettings(this), loginComponent(appSettings),
    openGLComponent(p, appSettings), selectorPanel(p, openGLComponent, appSettings),
    tvOverlayComponent(openGLComponent), launchRecorder("Export Video"), login("Login"), panelComponent(appSettings),
    videoComponent(openGLComponent), socketCueResolver(selectorPanel), globalSocketHandler(socketCueResolver)
{
    width = 1580;
    height = 670;
    setSize(width, height);

    resizeConstrainer = std::make_unique<juce::ComponentBoundsConstrainer>();
    resizeConstrainer->setSizeLimits(1283, 544, 2377, 1008);
    resizeConstrainer->setFixedAspectRatio(static_cast<double>(width) / static_cast<double>(height));;

    setResizable(true, false);
    setConstrainer(resizeConstrainer.get());
    centreWithSize(getWidth(), getHeight());

    addAndMakeVisible(openGLComponent);
    addAndMakeVisible(panelComponent);

    globalSocketHandler.startListening();
}

AudioVisualiserAudioProcessorEditor::~AudioVisualiserAudioProcessorEditor() {
    globalSocketHandler.destroy();
}

//==============================================================================
void AudioVisualiserAudioProcessorEditor::paint(juce::Graphics& g) {
    const float scale = getHeight() / 670.0f;

    g.fillAll(Theme::windowBg);

    // visualiser glowing border
    {
        const float thickness = juce::jmax(2.0f, 3.0f * scale);
        const float corner = 6.0f * scale;
        auto frame = visualiserArea.toFloat().expanded(thickness * 0.5f);

        for (int i = 3; i >= 1; --i)
        {
            g.setColour(Theme::accent.withAlpha(0.07f * (4 - i)));
            g.drawRect(frame.expanded(i * 1.5f * scale), 1.5f * scale);
        }

        juce::ColourGradient grad(Theme::accent, frame.getTopLeft(),
            Theme::accent2, frame.getBottomRight(), false);
        g.setGradientFill(grad);
        g.drawRect(frame, thickness);
    }

    // Right-hand control card
    g.setColour(Theme::card);
    g.fillRoundedRectangle(panelArea.toFloat(), 12.0f * scale);

    // panel cluster glowing border
    {
        const float thickness = juce::jmax(1.0f, 1.5f * scale);
        const float corner = 12.0f * scale;
        auto frame = panelArea.toFloat().expanded(thickness * 0.5f);

        for (int i = 3; i >= 1; --i)
        {
            g.setColour(Theme::accent.withAlpha(0.07f * (4 - i)));
            g.drawRoundedRectangle(frame.expanded(i * 1.5f * scale),
                corner + i * 1.5f * scale, 1.5f * scale);
        }

        juce::ColourGradient grad(Theme::accent, frame.getTopLeft(),
            Theme::accent2, frame.getBottomRight(), false);
        g.setGradientFill(grad);
        g.drawRoundedRectangle(frame, corner, thickness);
    }
}

void AudioVisualiserAudioProcessorEditor::resized() {
    const float scale = getHeight() / 670.0f;
    const int margin = juce::roundToInt(10 * scale);

    auto bounds = getLocalBounds();

    auto body = bounds.reduced(margin);

    // Visualiser: take the full body height first, keep the aspect ratio
    const float aspect = 815.0f / 460.0f;
    const int minPanelW = juce::roundToInt(170 * scale);

    int vh = body.getHeight();
    int vw = juce::roundToInt(vh * aspect);

    // Only fall back to width-limited if the panel would get too skinny
    if (body.getWidth() - vw - margin < minPanelW) {
        vw = body.getWidth() - margin - minPanelW;
        vh = juce::roundToInt(vw / aspect);
    }

    visualiserArea = juce::Rectangle<int>(body.getX(), 0, vw, vh).withCentre({ body.getX() + vw / 2, body.getCentreY() });
    openGLComponent.setBounds(visualiserArea);

    // Panel takes whatever width is left
    panelArea = body.withTrimmedLeft(vw + margin);

    panelComponent.setBounds(panelArea.reduced(juce::roundToInt(10 * scale)));
}