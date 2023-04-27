/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class EP491_DelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EP491_DelayAudioProcessorEditor (EP491_DelayAudioProcessor&);
    ~EP491_DelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using BoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    juce::Slider timeSlider;
    std::unique_ptr<SliderAttachment> timeAttachment;
    juce::Label timeLabel { "Time", "Time" };
    
    juce::Slider feedbackSlider, dwSlider;
    std::unique_ptr<SliderAttachment> feedbackAttachment, dwAttachment;
    juce::Label feedbackLabel { "Feedback", "Feedback" }, dwLabel { "Dry/Wet", "Dry/Wet" };
        
    void setSliderWithLabel (juce::Slider& slider, juce::Label& label, juce::AudioProcessorValueTreeState& apvts, juce::String paramID, std::unique_ptr<SliderAttachment>& attachment);
    
    EP491_DelayAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EP491_DelayAudioProcessorEditor)
};
