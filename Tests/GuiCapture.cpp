#include "PluginEditor.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
juce::Component* find(juce::Component& parent, const juce::String& id)
{
    if (parent.getComponentID() == id) return &parent;
    for (auto* child : parent.getChildren()) if (auto* result = find(*child, id)) return result;
    return nullptr;
}
template <typename T> T& control(juce::Component& parent, const juce::String& id)
{
    auto* result = dynamic_cast<T*>(find(parent, id));
    require(result != nullptr, "GUI control missing"); return *result;
}
void set(SPVinylLabProcessor& p, const juce::String& id, float value)
{
    auto* parameter = p.parameters.getParameter(id);
    require(parameter != nullptr, "Parameter missing");
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    parameter->endChangeGesture();
}
void save(SPVinylLabEditor& editor, const juce::File& file, float scale)
{
    const auto image = editor.createComponentSnapshot(editor.getLocalBounds(), true, scale);
    auto stream = file.createOutputStream();
    require(stream != nullptr, "Cannot create screenshot");
    stream->setPosition(0); stream->truncate();
    juce::PNGImageFormat png;
    require(png.writeImageToStream(image, *stream), "Cannot encode screenshot");
}
}
int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI initialiser;
    try {
        const auto directory = argc > 1 ? juce::File(juce::String::fromUTF8(argv[1]))
                                       : juce::File::getCurrentWorkingDirectory().getChildFile("GuiCaptures");
        require(directory.createDirectory().wasOk(), "Cannot create capture directory");
        SPVinylLabProcessor p;
        SPVinylLabEditor editor(p);
        editor.setVisible(true);
        auto& led = control<LEDDisplay>(editor, "ledDisplay");
        for (int mode = 0; mode < 3; ++mode) {
            set(p, "mode", static_cast<float>(mode)); editor.refreshFromProcessor();
            const auto& layout = VinylSkin::layout(mode);
            editor.setSize(layout.width, layout.height);
            for (int index = 0; index < 3; ++index) {
                auto& knob = control<juce::Slider>(editor, "primary" + juce::String(index));
                const char* names[] {mode == 0 ? "tone" : "compression", "noise", "wow"};
                const auto id = SPVinylLabProcessor::parameterID(mode, names[index]);
                for (float endpoint : {0.0f, 1.0f}) {
                    set(p, id, 1.0f - endpoint); // Ensure the following GUI update changes the value.
                    auto* parameter = p.parameters.getParameter(id);
                    parameter->beginChangeGesture();
                    knob.setValue(endpoint, juce::sendNotificationSync);
                    parameter->endChangeGesture();
                    require(std::abs(p.parameters.getRawParameterValue(id)->load() - endpoint) < 1.0e-6f, "Knob did not update APVTS");
                    require(led.getValue() == juce::roundToInt(endpoint * 127), "LED endpoint mismatch");
                }
                set(p, id, 32.0f / 127.0f);
                require(led.getValue() == 32, "Host automation did not update LED");
            }
            for (int index = 0; index < 2; ++index) {
                auto& slider = control<juce::Slider>(editor, index == 0 ? "inputGain" : "outputGain");
                const auto id = SPVinylLabProcessor::parameterID(mode, index == 0 ? "input" : "output");
                for (float value : {-24.0f, 0.0f, 12.0f}) {
                    auto* parameter = p.parameters.getParameter(id);
                    parameter->beginChangeGesture(); slider.setValue(value, juce::sendNotificationSync); parameter->endChangeGesture();
                    require(std::abs(p.parameters.getRawParameterValue(id)->load() - value) < 0.001f, "Fader did not update APVTS");
                }
            }
            auto& bypass = control<juce::ToggleButton>(editor, "bypass");
            bypass.setToggleState(true, juce::dontSendNotification); bypass.onClick();
            require(p.parameters.getRawParameterValue("enabled")->load() == 0, "Bypass polarity incorrect");
            set(p, "enabled", 1); require(!bypass.getToggleState(), "Bypass host update incorrect");
            // Seed controls with the values visible in each supplied reference.
            const int reference[3][3] {{127,18,98},{127,127,107},{0,21,26}};
            const char* names[] {mode == 0 ? "tone" : "compression", "noise", "wow"};
            for (int i = 0; i < 3; ++i)
                set(p, SPVinylLabProcessor::parameterID(mode, names[i]), static_cast<float>(reference[mode][i]) / 127.0f);
            const float input[] {-1.9f,-10.7f,-12.4f}, output[] {5.9f,10.1f,12.0f};
            set(p, SPVinylLabProcessor::parameterID(mode,"input"), input[mode]);
            set(p, SPVinylLabProcessor::parameterID(mode,"output"), output[mode]);
            // Select the same display source as the reference without changing its final value.
            const int selected = mode == 0 ? 0 : 1;
            const auto selectedID = SPVinylLabProcessor::parameterID(mode, names[selected]);
            const float value = static_cast<float>(reference[mode][selected]) / 127.0f;
            set(p, selectedID, value == 0 ? 0.01f : value - 0.001f); set(p, selectedID, value);
            p.prepareToPlay(48000, 512);
            juce::AudioBuffer<float> audio(2,512);
            for(int channel=0;channel<2;++channel)
                for(int sample=0;sample<512;++sample)
                    audio.setSample(channel,sample,0.5f*std::sin(static_cast<float>(sample)*0.071f));
            juce::MidiBuffer midi; p.processBlock(audio,midi);
            editor.refreshFromProcessor();
            const auto name = juce::StringArray{"4SX", "3DR", "40G"}[mode];
            save(editor, directory.getChildFile(name + "-1x.png"), 1);
            save(editor, directory.getChildFile(name + "-2x.png"), 2);
            editor.setSize(juce::roundToInt(layout.width * 1.5), juce::roundToInt(layout.height * 1.5));
            save(editor, directory.getChildFile(name + "-resized.png"), 1);
        }
        juce::MemoryBlock state; p.getStateInformation(state);
        SPVinylLabProcessor restored;
        restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        SPVinylLabEditor reopened(restored);
        require(control<juce::ComboBox>(reopened,"modeSelect").getSelectedItemIndex() == 2, "Restored skin mismatch");
        require(std::abs(control<juce::Slider>(reopened,"outputGain").getValue() - 12) < 0.001, "Restored gain mismatch");
        std::cout << "PASS: GUI bindings, LED 000/127, host updates, bypass, restored skin, 1x/2x/resized captures\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n'; return 1;
    }
}
