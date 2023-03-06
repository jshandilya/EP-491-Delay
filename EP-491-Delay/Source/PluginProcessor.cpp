/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EP491_DelayAudioProcessor::EP491_DelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParams())
#endif
{
//    mDelayTimeSmoothed = 0.f;
//    mCircularBufferLeft = nullptr;
//    mCircularBufferRight = nullptr;
//    mCircularBufferWriteHead = 0;
//    mCircularBufferLength = 0;
//    mDelayTimeInSamples = 0.f;
//    mDelayReadHead = 0.f;
//    mFeedbackLeft = 0.f;
//    mFeedbackRight = 0.f;
//    mDryWet = 0.5f;
    
    if (delayBuffer.hasBeenCleared() == false)
    {
        delayBuffer.clear();
    }
}

EP491_DelayAudioProcessor::~EP491_DelayAudioProcessor()
{
//    if (mCircularBufferLeft != nullptr) {
//        delete [] mCircularBufferLeft;
//        mCircularBufferLeft = nullptr;
//    }
//
//    if (mCircularBufferRight != nullptr) {
//        delete [] mCircularBufferRight;
//        mCircularBufferRight = nullptr;
//    }
    
    delayBuffer.clear();
}

//==============================================================================
const juce::String EP491_DelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EP491_DelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EP491_DelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EP491_DelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EP491_DelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EP491_DelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EP491_DelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EP491_DelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EP491_DelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void EP491_DelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EP491_DelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    auto delayBufferSize = sampleRate * 2.0;
    delayBuffer.setSize (getTotalNumOutputChannels(), (int)delayBufferSize);
    
    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
    {
        delayInMillis[ch].reset (sampleRate, 0.05f);
        feedback[ch].reset (sampleRate, 0.05f);
    }
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    filter.prepare (spec);
    
    reset();
}

void EP491_DelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EP491_DelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void EP491_DelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
//    auto& filterTypeProcess = *apvts.getRawParameterValue("FILTERTYPE");
//    auto& cutoff = *apvts.getRawParameterValue("FILTERFREQ");
//    auto& res = *apvts.getRawParameterValue("FILTERRES");

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        fillBuffer (buffer, channel);
        readFromBuffer (buffer, delayBuffer, channel);
        feedbackBuffer (buffer, channel);
    }
    
    updateBufferPositions (buffer, delayBuffer);
    
//    setType (filterTypeProcess);
//    filter.setCutoffFrequency (cutoff);
//    filter.setResonance (res);
//
//    auto audioBlock = juce::dsp::AudioBlock<float> (buffer);
//    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
//
//    filter.process (context);
}

void EP491_DelayAudioProcessor::fillBuffer (juce::AudioBuffer<float>& buffer, int channel)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();
    auto* wet = apvts.getRawParameterValue ("DRYWET");
    
    buffer.applyGain (0, bufferSize, 1.0f - (wet->load() / 100.0f));
    
    // Check to see if main buffer copies to delay buffer without needing to wrap...
    if (delayBufferSize >= bufferSize + writePosition)
    {
        // copy main buffer contents to delay buffer
        delayBuffer.copyFrom (channel, writePosition, buffer.getWritePointer (channel), bufferSize);
    }
    // if no
    else
    {
        // Determine how much space is left at the end of the delay buffer
        auto numSamplesToEnd = delayBufferSize - writePosition;
        
        // Copy that amount of contents to the end...
        delayBuffer.copyFrom (channel, writePosition, buffer.getWritePointer (channel), numSamplesToEnd);
        
        // Calculate how much contents is remaining to copy
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        
        // Copy remaining amount to beginning of delay buffer
        delayBuffer.copyFrom (channel, 0, buffer.getWritePointer (channel, numSamplesToEnd), numSamplesAtStart);
    }
}

void EP491_DelayAudioProcessor::feedbackBuffer (juce::AudioBuffer<float>& buffer, int channel)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();
    
    // feedback
    
    auto fb = apvts.getRawParameterValue ("FEEDBACK")->load();
    
    // Check to see if main buffer copies to delay buffer without needing to wrap...
    if (delayBufferSize >= bufferSize + writePosition)
    {
        // copy main buffer contents to delay buffer
        delayBuffer.addFromWithRamp (channel, writePosition, buffer.getWritePointer (channel), bufferSize, fb, fb);
    }
    // if no
    else
    {
        // Determine how much space is left at the end of the delay buffer
        auto numSamplesToEnd = delayBufferSize - writePosition;
        
        // Copy that amount of contents to the end...
        delayBuffer.addFromWithRamp (channel, writePosition, buffer.getWritePointer (channel), numSamplesToEnd, fb, fb);
        
        // Calculate how much contents is remaining to copy
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        
        // Copy remaining amount to beginning of delay buffer
        delayBuffer.addFromWithRamp (channel, 0, buffer.getWritePointer (channel, numSamplesToEnd), numSamplesAtStart, fb, fb);
    }
}

void EP491_DelayAudioProcessor::readFromBuffer (juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer, int channel)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();
    
    auto percent = apvts.getRawParameterValue ("DRYWET")->load();
    
    auto g = juce::jmap (percent, 0.0f, 100.0f, 0.0f, 1.0f);
    
//    auto g = 0.5f;
    
    g += 0.5;
        
//    auto dryGain = 5.0f - g;
    
    auto delayTimeLeft = apvts.getRawParameterValue ("DELAYMSLEFT")->load();
    auto delayTimeRight = apvts.getRawParameterValue ("DELAYMSRIGHT")->load();
    
    if (apvts.getRawParameterValue ("DELAYLINK")->load() == 0)
    {
        delayTimeRight = delayTimeLeft;
    }
    
    auto delayTime = channel == 0 ? delayTimeLeft : delayTimeRight;
    
    // delayMs
    auto readPosition = std::round (writePosition - (getSampleRate() * delayTime / 1000.0f));
    
    if (readPosition < 0)
        readPosition += delayBufferSize;
    
//    buffer.applyGainRamp (0, bufferSize, dryGain, dryGain);
    
    if (readPosition + bufferSize < delayBufferSize)
    {
        buffer.addFromWithRamp (channel, 0, delayBuffer.getReadPointer (channel, readPosition), bufferSize, g, g);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - readPosition;
        buffer.addFromWithRamp (channel, 0, delayBuffer.getReadPointer (channel, readPosition), numSamplesToEnd, g, g);
        
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        buffer.addFromWithRamp (channel, numSamplesToEnd, delayBuffer.getReadPointer (channel, 0), numSamplesAtStart, g, g);
    }
}

void EP491_DelayAudioProcessor::updateBufferPositions (juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();
    
    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}


//==============================================================================
bool EP491_DelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EP491_DelayAudioProcessor::createEditor()
{
    return new EP491_DelayAudioProcessorEditor (*this);
}

//==============================================================================
void EP491_DelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void EP491_DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EP491_DelayAudioProcessor();
}

float EP491_DelayAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
}

void EP491_DelayAudioProcessor::reset()
{
    filter.reset();
}

void EP491_DelayAudioProcessor::setType(int choice)
{
    using fType = juce::dsp::StateVariableTPTFilterType;
    
    switch (choice)
    {
        case 0:
            filter.setType (fType::lowpass);
            break;
            
        case 1:
            filter.setType (fType::bandpass);
            break;
            
        case 2:
            filter.setType (fType::highpass);
            break;
            
        default:
            break;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout EP491_DelayAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>("DELAYMSLEFT", "Delay Ms Left", 0.0f, 2000.0f, 500.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("DELAYMSRIGHT", "Delay Ms Right", 0.0f, 2000.0f, 350.0f));
    
//    params.push_back (std::make_unique<juce::AudioParameterBool>("DELAYLINK", "Delay Link", false));
    
    params.push_back (std::make_unique<juce::AudioParameterChoice>("DELAYLINK", "Delay Link", juce::StringArray { "Linked", "Unlinked" }, 0));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>("FEEDBACK", "Feedback", 0.0f, 0.9f, 0.5f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>("DRYWET", "Dry/Wet", 0.0f, 100.0f, 60.0f));
    
//    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "FILTERTYPE", 1 }, "Filter Type", juce::StringArray { "Lowpass", "Bandpass", "Highpass" }, 0));
//
//    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FILTERFREQ", 1}, "Filter Freq", juce::NormalisableRange<float> { 20.0f, 20000.0f, 0.1f, 0.6f }, 20000.0f));
//
//    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FILTERRES", 1}, "Filter Resonance", juce::NormalisableRange<float> { 1.0f, 10.0f, 0.01f}, 1.0f));
    
    return { params.begin(), params.end() };
}
