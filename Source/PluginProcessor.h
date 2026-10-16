#pragma once
#include <array>
#include <atomic>
#include <memory>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/VinylNoise.h"
#include "DSP/WowFlutter.h"
#include "DSP/VintageCompressor.h"
#include "DSP/FrequencyRange.h"
#include "DSP/Saturation.h"

class SPVinylLabProcessor final : public juce::AudioProcessor
{
public:
    SPVinylLabProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "SP Vinyl Lab"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    static juce::String parameterID(int mode, const juce::String& name);
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> peak { 0.0f };
    std::atomic<float> inputPeak { 0.0f }, outputPeak { 0.0f };
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    std::atomic<float>* modeValue = nullptr;
    std::atomic<float>* enabledValue = nullptr;
    std::array<std::array<std::atomic<float>*, 6>, 3> values {};
    std::array<std::array<std::atomic<float>*, 2>, 3> gainValues {};
    VinylNoise noise;
    WowFlutter wow;
    VintageCompressor compressor;
    FrequencyRange filter;
    Saturation saturation;
    juce::SmoothedValue<float> noiseAmount, wowAmount, compression, tone, drive, mix;
    juce::SmoothedValue<float> inputGain, outputGain;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPVinylLabProcessor)
};
