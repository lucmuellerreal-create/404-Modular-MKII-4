#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class LevelMeter final : public juce::Component {
public:
    void setSkin(int value) { skin=value;repaint(); }
    void advance(float target,float seconds);
    void paint(juce::Graphics&) override;
private:
    float level=0;
    int skin=0;
};
