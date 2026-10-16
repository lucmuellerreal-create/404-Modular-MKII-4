#pragma once
#include "PluginProcessor.h"
#include "GUI/SkinDesign.h"
#include "GUI/VinylLookAndFeel.h"
#include "GUI/VintageKnob.h"
#include "GUI/LEDDisplay.h"
#include "GUI/LevelMeter.h"
class SPVinylLabEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit SPVinylLabEditor(SPVinylLabProcessor&);
    ~SPVinylLabEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    // Also used by the offscreen capture/test target on the message thread.
    void refreshFromProcessor();
private:
    void timerCallback() override;
    void attachMode(int);
    void updateReadouts();
    SPVinylLabProcessor& processor;
    VinylLookAndFeel look;
    VinylPanel panel;
    juce::ComboBox modes;
    juce::ToggleButton bypass {"Bypass"};
    LEDDisplay display;
    LevelMeter inputMeter, outputMeter;
    std::array<VintageKnob, 3> knobs;
    std::array<juce::Label, 3> values;
    std::array<juce::Slider, 2> gains;
    std::array<juce::Label, 2> gainValues;
    juce::Label displayCaption;
    using APVTS = juce::AudioProcessorValueTreeState;
    std::unique_ptr<APVTS::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::ParameterAttachment> enabledAttachment;
    std::array<std::unique_ptr<APVTS::SliderAttachment>, 3> attachments;
    std::array<std::unique_ptr<APVTS::SliderAttachment>, 2> gainAttachments;
    std::array<int, 3> selectedControl {{0,0,0}};
    int attachedMode = -1, activeDrags = 0;
    bool rebinding = false;
    float peakHold = 0;
    double previousTick = 0;
    juce::TooltipWindow tooltips {this,700};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPVinylLabEditor)
};
