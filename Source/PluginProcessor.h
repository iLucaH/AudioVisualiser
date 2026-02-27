/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RingBuffer.h"

//==============================================================================
/**
*/
class AudioVisualiserAudioProcessor  : public juce::AudioProcessor, public juce::ChangeListener
{
public:
    //==============================================================================
    AudioVisualiserAudioProcessor();
    ~AudioVisualiserAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    enum TransportState {
        Stopped,
        Starting,
        Stopping,
        Playing
    };

    void setNewTransportSource(juce::File& file) {
        // Make sure that this executes safely on the audio thread.
        juce::ScopedLock lock(getCallbackLock());

        // Prepare the transport.
        transport.stop();
        transport.setSource(nullptr);
        playSource.reset();

        juce:: AudioFormatReader* reader = formatManager.createReaderFor(file);
        if (reader != nullptr) {
            // Get the file ready to play.
            std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));

            transport.setSource(tempSource.get(), 0, nullptr, reader->sampleRate);
            transportStateChanged(Stopped);

            playSource.reset(tempSource.release());

            transportStateChanged(Stopped);
        }
    }

    void transportStateChanged(TransportState newState) {
        // Make sure that this executes safely on the audio thread.
        juce::ScopedLock lock(getCallbackLock());

        if (newState != state) {
            state = newState;

            switch (state) {
            case Stopped:
                isRawInput = true;
                transport.setPosition(0.0);
                break;

            case Playing:
                isRawInput = false;
                break;

            case Starting:
                isRawInput = false;
                transport.start();
                break;

            case Stopping:
                isRawInput = false;
                transport.stop();
                break;
            }
        }
    }

    float getRMS(int channel) {
        jassert(channel == 0 || channel == 1);
        return channel == 0 ? leftRMS : rightRMS;
    }

    RingBuffer<float>& getRingBuffer() {
        return *ringBuffer;
    }



private:
    std::unique_ptr<RingBuffer<float>> ringBuffer;
    float leftRMS = 0;
    float rightRMS = 0;

    bool isRawInput = true; // Whether the input is a raw block of audio thats coming in or if the audio should come from the custom transport source.
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    juce::AudioTransportSource transport;

    TransportState state;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioVisualiserAudioProcessor)
};
