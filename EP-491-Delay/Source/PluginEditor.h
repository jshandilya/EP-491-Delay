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
    
    juce::ComboBox linkBox;
    std::unique_ptr<BoxAttachment> linkBoxAttachment;
    juce::Label linkLabel { "Delay Link", "Link" };
    
    juce::Slider delayLeftSlider, delayRightSlider;
    std::unique_ptr<SliderAttachment> delayLeftAttachment, delayRightAttachment;
    juce::Label delayLeftLabel { "Delay Time Main/Left", "Main/Left" }, delayRightLabel { "Delay Time Right", "Right" };
    
    juce::Slider feedbackSlider, dwSlider;
    std::unique_ptr<SliderAttachment> feedbackAttachment, dwAttachment;
    juce::Label feedbackLabel { "Feedback", "Feedback" }, dwLabel { "Dry/Wet", "Dry/Wet" };
    
    void setComboBox();
    
    void setSliderWithLabel (juce::Slider& slider, juce::Label& label, juce::AudioProcessorValueTreeState& apvts, juce::String paramID, std::unique_ptr<SliderAttachment>& attachment);
    
    EP491_DelayAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EP491_DelayAudioProcessorEditor)
};
