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
    
    setSize (400, 360);
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
    const auto padding = 40;
    
    const auto startX = 10;
    const auto startY = 55;
    
    const auto boxWidth = 90;
    const auto boxHeight = 30;
    
    const auto sliderWidth = 100;
    const auto sliderHeight = 100;
    
    const auto labelYOffset = 20;
    const auto labelHeight = 20;    
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
