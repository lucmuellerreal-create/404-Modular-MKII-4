#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class LEDDisplay final : public juce::Component {
public:
    void setValue(int v) { v=juce::jlimit(0,127,v);if(value!=v){value=v;repaint();} }
    int getValue() const noexcept { return value; }
    void paint(juce::Graphics&) override;
private:
    int value=0;
};
