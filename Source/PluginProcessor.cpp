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

    // 1. Copy audio into FFT buffer
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    const int samplesToCopy = juce::jmin(fftSize, buffer.getNumSamples());

    for (int i = 0; i < samplesToCopy; ++i) {
        fftData[i] = buffer.getSample(0, i);
    }

    // Zero the imaginary portion required by JUCE
    std::fill(fftData.begin() + fftSize, fftData.end(), 0.0f);

    // Perform FFT
    fft.performRealOnlyForwardTransform(fftData.data());

    // Convert FFT output to magnitude
    std::array<float, fftSize / 2> magnitudes{};

    for (int i = 0; i < fftSize / 2; ++i) {
        const float real = fftData[i * 2];
        const float imag = fftData[i * 2 + 1];

        magnitudes[i] = std::sqrt(real * real + imag * imag);
    }

    // Convert to 128 logarithmically-spaced frequency bands
    constexpr float minFrequency = 20.0f;
    const float maxFrequency = getSampleRate() * 0.5f;

    const float fftBinWidth = getSampleRate() / static_cast<float>(fftSize);

    for (int i = 0; i < numShaderBins; ++i) {
        // Position of this shader bin: 0 -> 1
        const float t0 = static_cast<float>(i) / static_cast<float>(numShaderBins);
        const float t1 = static_cast<float>(i + 1) / static_cast<float>(numShaderBins);

        // Logarithmic frequency boundaries
        const float frequencyStart = minFrequency * std::pow(maxFrequency / minFrequency, t0);
        const float frequencyEnd = minFrequency * std::pow(maxFrequency / minFrequency, t1);

        // Convert frequencies to FFT bins
        int startBin = static_cast<int>(frequencyStart / fftBinWidth);
        int endBin = static_cast<int>(frequencyEnd / fftBinWidth);

        startBin = juce::jlimit(0, fftSize / 2 - 1, startBin);
        endBin = juce::jlimit(startBin + 1, fftSize / 2, endBin);

        // Average all FFT bins inside this logarithmic band
        float sum = 0.0f;
        int count = 0;

        for (int bin = startBin; bin < endBin; ++bin) {
            sum += magnitudes[bin];
            ++count;
        }

        shaderFFT[i] = count > 0 ? sum / static_cast<float>(count) : 0.0f;
    }

    // Convert magnitude to dB and normalise between 0 and 1
    for (auto& value : shaderFFT)
    {
        value /= static_cast<float>(fftSize);

        value = juce::Decibels::gainToDecibels(value, -100.0f);
        value = juce::jmap(value, -100.0f, 0.0f, 0.0f, 1.0f);
        value = juce::jlimit(0.0f, 1.0f, value);
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
