#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class VinylLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    VinylLookAndFeel();
    void setSkin(int value) { skin = value; }
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
private:
    int skin = 0;
};
