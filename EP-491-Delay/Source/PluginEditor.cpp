/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EP491_DelayAudioProcessorEditor::EP491_DelayAudioProcessorEditor (EP491_DelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSliderWithLabel (timeSlider, timeLabel, audioProcessor.apvts, "TIME", timeAttachment);
    setSliderWithLabel (feedbackSlider, feedbackLabel, audioProcessor.apvts, "FEEDBACK", feedbackAttachment);
    setSliderWithLabel (dwSlider, dwLabel, audioProcessor.apvts, "DRYWET", dwAttachment);
    
    setSliderWithLabel (reverbSizeSlider, reverbSizeLabel, audioProcessor.apvts, "REVERBSIZE", reverbSizeAttachment);
    setSliderWithLabel (reverbWidthSlider, reverbWidthLabel, audioProcessor.apvts, "REVERBWIDTH", reverbWidthAttachment);
    setSliderWithLabel (reverbDampSlider, reverbDampLabel, audioProcessor.apvts, "REVERBDAMPING", reverbDampAttachment);
    setSliderWithLabel (reverbDrySlider, reverbDryLabel, audioProcessor.apvts, "REVERBDRY", reverbDryAttachment);
    setSliderWithLabel (reverbWetSlider, reverbWetLabel, audioProcessor.apvts, "REVERBWET", reverbWetAttachment);
    setSliderWithLabel (reverbFreezeSlider, reverbFreezeLabel, audioProcessor.apvts, "REVERBFREEZE", reverbFreezeAttachment);
    
    setSize (650, 400);
}

EP491_DelayAudioProcessorEditor::~EP491_DelayAudioProcessorEditor()
{
}

//==============================================================================
void EP491_DelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

}

void EP491_DelayAudioProcessorEditor::resized()
{
    const auto sliderWidth = 125;
    const auto sliderHeight = 125;
    
    const auto sliderWidth2 = 100;
    const auto sliderHeight2 = 100;
    
    const auto sliderY2 = 225;
    
    const auto labelYOffset = 20;
    const auto labelHeight = 20;
    
    timeSlider.setBounds (155, 50, sliderWidth, sliderHeight);
    timeLabel.setBounds (timeSlider.getX(), timeSlider.getY() - labelYOffset, timeSlider.getWidth(), labelHeight);
    
    feedbackSlider.setBounds (265, 50, sliderWidth, sliderHeight);
    feedbackLabel.setBounds (feedbackSlider.getX(), feedbackSlider.getY() - labelYOffset, feedbackSlider.getWidth(), labelHeight);
    
    dwSlider.setBounds (375, 50, sliderWidth, sliderHeight);
    dwLabel.setBounds (dwSlider.getX(), dwSlider.getY() - labelYOffset, dwSlider.getWidth(), labelHeight);
    
    reverbSizeSlider.setBounds (25, sliderY2, sliderWidth2, sliderHeight2);
    reverbSizeLabel.setBounds (reverbSizeSlider.getX(), reverbSizeSlider.getY() - labelYOffset, reverbSizeSlider.getWidth(), labelHeight);
    
    reverbWidthSlider.setBounds (125, sliderY2, sliderWidth2, sliderHeight2);
    reverbWidthLabel.setBounds (reverbWidthSlider.getX(), reverbWidthSlider.getY() - labelYOffset, reverbWidthSlider.getWidth(), labelHeight);
    
    reverbDampSlider.setBounds (225, sliderY2, sliderWidth2, sliderHeight2);
    reverbDampLabel.setBounds (reverbDampSlider.getX(), reverbDampSlider.getY() - labelYOffset, reverbDampSlider.getWidth(), labelHeight);
    
    reverbDrySlider.setBounds (325, sliderY2, sliderWidth2, sliderHeight2);
    reverbDryLabel.setBounds (reverbDrySlider.getX(), reverbDrySlider.getY() - labelYOffset, reverbDrySlider.getWidth(), labelHeight);
    
    reverbWetSlider.setBounds (425, sliderY2, sliderWidth2, sliderHeight2);
    reverbWetLabel.setBounds (reverbWetSlider.getX(), reverbWetSlider.getY() - labelYOffset, reverbWetSlider.getWidth(), labelHeight);
    
    reverbFreezeSlider.setBounds (525, sliderY2, sliderWidth2, sliderHeight2);
    reverbFreezeLabel.setBounds (reverbFreezeSlider.getX(), reverbFreezeSlider.getY() - labelYOffset, reverbFreezeSlider.getWidth(), labelHeight);
}

void EP491_DelayAudioProcessorEditor::setSliderWithLabel (juce::Slider& slider, juce::Label& label, juce::AudioProcessorValueTreeState& apvts, juce::String paramID, std::unique_ptr<SliderAttachment>& attachment)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
    addAndMakeVisible(slider);
    
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
    
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void EP491_DelayAudioProcessorEditor::setReverbSliderWithLabel (juce::Slider& slider, juce::Label& label, juce::AudioProcessorValueTreeState& apvts, juce::String paramID, std::unique_ptr<SliderAttachment>& attachment)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
    addAndMakeVisible(slider);
    
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
    
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}
