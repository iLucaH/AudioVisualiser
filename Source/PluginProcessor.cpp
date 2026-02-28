/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioVisualiserAudioProcessor::AudioVisualiserAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    ringBuffer = std::make_unique<RingBuffer<float>>(2, 32768); // 32768 covers hopefully all sample sizes;
    formatManager.registerBasicFormats();
    transport.addChangeListener(this);
}

AudioVisualiserAudioProcessor::~AudioVisualiserAudioProcessor()
{
}

//==============================================================================
const juce::String AudioVisualiserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioVisualiserAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioVisualiserAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioVisualiserAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioVisualiserAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioVisualiserAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioVisualiserAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioVisualiserAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AudioVisualiserAudioProcessor::getProgramName (int index)
{
    return {};
}

void AudioVisualiserAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AudioVisualiserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // Can no longer initialise ringBuffer here because of a race condition that occurs when ringBuffer is changing sizes due to a new audio source,
    // but the other threads are still trying to take from the ring buffer.
    // ringBuffer = std::make_unique<RingBuffer<float>>(2, samplesPerBlock * 10); // multiply by 10 to allow extra room. SamplesPerBlock is not a guarenteed number.
    transport.prepareToPlay(samplesPerBlock, sampleRate);
}

void AudioVisualiserAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    transport.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioVisualiserAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AudioVisualiserAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (isRawInput) {
        leftRMS = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
        rightRMS = buffer.getNumChannels() > 1 ? buffer.getRMSLevel(1, 0, buffer.getNumSamples()) : leftRMS;

        ringBuffer->writeSamples(buffer, 0, buffer.getNumSamples());
    } else {
        // Wrap the buffer
        juce::AudioSourceChannelInfo bufferToFill(&buffer, 0, buffer.getNumSamples());

        transport.getNextAudioBlock(bufferToFill);

        leftRMS = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
        rightRMS = buffer.getNumChannels() > 1 ? buffer.getRMSLevel(1, 0, buffer.getNumSamples()) : leftRMS;

        ringBuffer->writeSamples(buffer, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool AudioVisualiserAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioVisualiserAudioProcessor::createEditor()
{
    return new AudioVisualiserAudioProcessorEditor (*this);
}

//==============================================================================
void AudioVisualiserAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AudioVisualiserAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioVisualiserAudioProcessor();
}
