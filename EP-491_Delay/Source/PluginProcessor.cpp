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
                       ), apvts(*this, nullptr, "Parameters", createParams())
#endif
{
//    addParameter(mDryWetParameter = new juce::AudioParameterFloat("drywet", "Dry Wet", 0.0, 1.0, 0.5));
//
//    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback", "Feedback", 0, 0.98, 0.5));
//
//    addParameter(mDelayTimeParameter = new juce::AudioParameterFloat("delaytime","Delay Time", 0.01, MAX_DELAY_TIME, 0.5));
    
    mDelayTimeSmoothed = 0;
    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    mDelayTimeInSamples = 0;
    mDelayReadHead = 0;
    mFeedbackLeft = 0;
    mFeedbackRight = 0;
    mDryWet = 0.5f;
}

EP491_DelayAudioProcessor::~EP491_DelayAudioProcessor()
{
    if (mCircularBufferLeft != nullptr) {
        delete [] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;

    }
    
    if (mCircularBufferRight != nullptr) {
        delete [] mCircularBufferRight;
        mCircularBufferRight = nullptr;
    }
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
    auto& delayTime = *apvts.getRawParameterValue("TIME");
    
//    mDelayTimeInSamples = sampleRate * *mDelayTimeParameter;
    mDelayTimeInSamples = sampleRate * delayTime;

    
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
    
    if (mCircularBufferLeft != nullptr) {
        delete [] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;
    }

    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float[(int)(mCircularBufferLength)];
    }

    juce::zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));

    if (mCircularBufferRight != nullptr) {
        delete [] mCircularBufferRight;
        mCircularBufferRight = nullptr;
    }

    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[(int)(mCircularBufferLength)];
    }

    juce::zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));
    
    mCircularBufferWriteHead = 0;

    mDelayTimeSmoothed = delayTime;
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

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    auto& delayTime = *apvts.getRawParameterValue("TIME");
    auto& delayFeedback = *apvts.getRawParameterValue("FEEDBACK");
    auto& delayDryWet = *apvts.getRawParameterValue("DRYWET");
    
//    mDelayTimeInSamples = getSampleRate() * *mDelayTimeParameter;
    mDelayTimeInSamples = getSampleRate() * delayTime;

    
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        
//        mDelayTimeSmoothed = mDelayTimeSmoothed - 0.001 * (mDelayTimeSmoothed - *mDelayTimeParameter);
        mDelayTimeSmoothed = mDelayTimeSmoothed - 0.001 * (mDelayTimeSmoothed - delayTime);

        mDelayTimeInSamples = getSampleRate() * mDelayTimeSmoothed;
        
        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;
        
        mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;
        
        if (mDelayReadHead < 0) {
            mDelayReadHead += mCircularBufferLength;
        }
        
        int readHead_x = (int)mDelayReadHead;
        int readHead_x1 = readHead_x + 1;
        float readHeadFloat = mDelayReadHead - readHead_x;

        if (readHead_x1 >= mCircularBufferLength) {
            readHead_x1 -= mCircularBufferLength;
        }
        
        float delay_sample_left = lin_interp(mCircularBufferLeft[readHead_x], mCircularBufferLeft[readHead_x1], readHeadFloat);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHead_x], mCircularBufferRight[readHead_x1], readHeadFloat);
        
//        mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedbackLeft = delay_sample_left * delayFeedback;
        
//        mFeedbackRight = delay_sample_right * *mFeedbackParameter;
        mFeedbackRight = delay_sample_right * delayFeedback;

        
        
        mCircularBufferWriteHead++;
        
//        buffer.setSample(0, i, buffer.getSample(0, i) * (1 - *mDryWetParameter) + delay_sample_left * *mDryWetParameter);
        buffer.setSample(0, i, buffer.getSample(0, i) * (1 - *mDryWetParameter) + delay_sample_left * delayDryWet);

        
//        buffer.setSample(1, i, buffer.getSample(1, i) * (1 - *mDryWetParameter) + delay_sample_right * *mDryWetParameter);
        buffer.setSample(1, i, buffer.getSample(1, i) * (1 - *mDryWetParameter) + delay_sample_right * delayDryWet);
        
        if (mCircularBufferWriteHead >= mCircularBufferLength) {
            mCircularBufferWriteHead = 0;
        }
    }
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EP491_DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
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

juce::AudioProcessorValueTreeState::ParameterLayout EP491_DelayAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
//    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID { "VOICES", 1 }, "Voices", 1, 8, 8));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "TIME", 1 }, "Time", 0.01f, MAX_DELAY_TIME, 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FEEDBACK", 1 }, "Feedback", 0.0f, 0.98f, 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "DRYWET", 1 }, "Dry Wet", 0.0f, 1.0f, 0.5f));
    
    return { params.begin(), params.end() };
}
