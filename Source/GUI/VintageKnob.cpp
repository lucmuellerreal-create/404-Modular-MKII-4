#include "VintageKnob.h"
VintageKnob::VintageKnob()
{
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);
    setMouseDragSensitivity(220);
    setVelocityBasedMode(false);
    setWantsKeyboardFocus(true);
    setPopupDisplayEnabled(false,false,nullptr);
    textFromValueFunction=[](double v){return juce::String(juce::roundToInt(v*127.0));};
    valueFromTextFunction=[](const juce::String& t){return juce::jlimit(0.0,1.0,t.getDoubleValue()/127.0);};
}
