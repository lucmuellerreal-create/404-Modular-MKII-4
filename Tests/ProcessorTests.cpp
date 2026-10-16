#include "PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
void set(SPVinylLabProcessor& p, const juce::String& id, float raw)
{
    auto* parameter = p.parameters.getParameter(id);
    require(parameter != nullptr, "Parameter missing");
    parameter->setValueNotifyingHost(parameter->convertTo0to1(raw));
}
void fill(juce::AudioBuffer<float>& b)
{
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < b.getNumSamples(); ++i)
            b.setSample(c, i, 0.8f * std::sin(static_cast<float>(i * (c + 1)) * 0.071f));
}
void compare(const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b)
{
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < a.getNumSamples(); ++i)
            require(a.getSample(c, i) == b.getSample(c, i), "Passthrough changed a sample");
}
}
int main()
{
    juce::ScopedJuceInitialiser_GUI initialiser;
    try {
        SPVinylLabProcessor p;
        require(p.getTotalNumInputChannels() == 2 && p.getTotalNumOutputChannels() == 2, "Stereo bus missing");
        auto mono = p.getBusesLayout();
        mono.inputBuses.set(0, juce::AudioChannelSet::mono());
        mono.outputBuses.set(0, juce::AudioChannelSet::mono());
        require(!p.isBusesLayoutSupported(mono), "Mono should be rejected");
        require(p.getParameters().size() == 26, "Expected 26 host parameters");
        juce::MidiBuffer midi;
        for (double sr : {44100.0, 48000.0, 96000.0, 192000.0}) {
            p.prepareToPlay(sr, 1024);
            for (int size : {0, 1, 17, 64, 511, 1024}) {
                juce::AudioBuffer<float> b(2, size); fill(b);
                juce::AudioBuffer<float> original; original.makeCopyOf(b);
                p.processBlock(b, midi); compare(b, original);
            }
        }
        for (int m = 0; m < 3; ++m) {
            set(p, SPVinylLabProcessor::parameterID(m, "noise"), 0.2f + static_cast<float>(m) * 0.25f);
            set(p, SPVinylLabProcessor::parameterID(m, "input"), -9.0f + static_cast<float>(m));
            set(p, SPVinylLabProcessor::parameterID(m, "output"), 3.0f + static_cast<float>(m));
        }
        set(p, "mode", 2.0f); set(p, "enabled", 1.0f);
        juce::MemoryBlock state; p.getStateInformation(state);
        SPVinylLabProcessor restored; restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        for (auto* base : p.getParameters()) {
            auto* parameter = dynamic_cast<juce::RangedAudioParameter*>(base);
            require(parameter != nullptr, "Unexpected parameter type");
            auto* other = restored.parameters.getParameter(parameter->paramID);
            require(other != nullptr && std::abs(other->getValue() - parameter->getValue()) < 1.0e-6f, "State roundtrip failed");
        }
        const char invalid[] = "invalid state";
        restored.setStateInformation(invalid, sizeof(invalid));
        require(restored.parameters.getRawParameterValue("mode")->load() == 2.0f, "Invalid state changed mode");
        for (int m = 0; m < 3; ++m) {
            set(restored, "mode", static_cast<float>(m));
            restored.prepareToPlay(48000, 511);
            for (int block = 0; block < 100; ++block) {
                juce::AudioBuffer<float> b(2, 511); fill(b); restored.processBlock(b, midi);
                for (int c = 0; c < 2; ++c)
                    for (int i = 0; i < b.getNumSamples(); ++i)
                        require(std::isfinite(b.getSample(c, i)), "DSP produced nonfinite output");
            }
        }
        set(restored, SPVinylLabProcessor::parameterID(2, "mix"), 0.0f);
        set(restored, SPVinylLabProcessor::parameterID(2, "input"), 0.0f);
        set(restored, SPVinylLabProcessor::parameterID(2, "output"), 0.0f);
        set(restored, "mode", 2); restored.prepareToPlay(48000, 64);
        juce::AudioBuffer<float> b(2, 64); fill(b);
        juce::AudioBuffer<float> original; original.makeCopyOf(b);
        restored.processBlock(b, midi); compare(b, original);
        set(restored, "m2_input", -6.0f); set(restored, "m2_output", -6.0f);
        restored.prepareToPlay(48000, 64); fill(b);
        restored.processBlock(b, midi);
        const float expectedGain = juce::Decibels::decibelsToGain(-12.0f);
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < b.getNumSamples(); ++i)
                require(std::abs(b.getSample(c, i) - original.getSample(c, i) * expectedGain) < 1.0e-6f, "Gain fader did not affect audio");
        set(restored, "enabled", 0.0f); fill(b);
        restored.processBlock(b, midi); compare(b, original);
        auto legacyState = restored.parameters.copyState();
        for (int m = 0; m < 3; ++m)
            for (const auto* name : {"input", "output"})
                legacyState.removeChild(legacyState.getChildWithProperty("id", SPVinylLabProcessor::parameterID(m, name)), nullptr);
        auto legacyXML = legacyState.createXml();
        require(legacyXML != nullptr, "Cannot create legacy state");
        juce::MemoryBlock legacyBlock;
        juce::AudioProcessor::copyXmlToBinary(*legacyXML, legacyBlock);
        restored.setStateInformation(legacyBlock.getData(), static_cast<int>(legacyBlock.getSize()));
        require(restored.parameters.getRawParameterValue("m2_input")->load() == 0 && restored.parameters.getRawParameterValue("m2_output")->load() == 0, "Legacy project did not restore unity gains");
        std::cout << "PASS: stereo, exact passthrough, state, invalid state, DSP, dry mix\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n'; return 1;
    }
}
