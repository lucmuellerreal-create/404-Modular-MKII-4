#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace VinylSkin {
struct Layout {
    int width, height;
    juce::Rectangle<float> shell, top, bottom, ring;
    std::array<juce::Rectangle<int>, 3> knobs;
    juce::Rectangle<int> display, input, output, inputMeter, outputMeter, bypass, mode;
    juce::Point<float> peak, effect;
};
const Layout& layout(int mode);
juce::Colour ink(int mode);
void text(juce::Graphics&, const juce::String&, juce::Rectangle<float>, float size,
          juce::Colour, bool bold = true);
void lamp(juce::Graphics&, juce::Point<float>, bool, juce::Colour);
void paint(juce::Graphics&, int mode, bool effect, bool peak);
}

class VinylPanel final : public juce::Component {
public:
    void setMode(int value) { mode = value; repaint(); }
    void setIndicators(bool active, bool clipped) {
        if (effect != active || peak != clipped) {
            effect = active; peak = clipped;
            const auto& l = VinylSkin::layout(mode);
            repaint(juce::Rectangle<int>(static_cast<int>(l.effect.x) - 9, static_cast<int>(l.effect.y) - 9, 18, 18));
            repaint(juce::Rectangle<int>(static_cast<int>(l.peak.x) - 9, static_cast<int>(l.peak.y) - 9, 18, 18));
        }
    }
    void paint(juce::Graphics& g) override { VinylSkin::paint(g, mode, effect, peak); }
private:
    int mode = 0;
    bool effect = false, peak = false;
};
