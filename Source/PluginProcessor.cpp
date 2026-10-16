#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace { constexpr const char* names[] { "noise", "wow", "compression", "tone", "drive", "mix" }; }
juce::String SPVinylLabProcessor::parameterID(int mode, const juce::String& name)
{ return "m" + juce::String(mode) + "_" + name; }
juce::AudioProcessorValueTreeState::ParameterLayout SPVinylLabProcessor::makeParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"mode", 1}, "Vinyl Mode", juce::StringArray{"4SX Vinyl", "3DR Vinyl", "40G Vinyl"}, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"enabled", 1}, "Vinyl Processing", false));
    const float defaults[3][6] {{0.12f,0.15f,0.3f,0.7f,0.2f,1.0f}, {0.2f,0.3f,0.45f,0.5f,0.35f,1.0f}, {0.08f,0.1f,0.2f,0.85f,0.12f,1.0f}};
    for (int m = 0; m < 3; ++m)
        for (int p = 0; p < 6; ++p)
            layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{parameterID(m, names[p]), 1}, juce::StringArray{"4SX", "3DR", "40G"}[m] + " " + names[p], juce::NormalisableRange<float>{0.0f, 1.0f}, defaults[m][p]));
    // Append new parameters, preserving the order and IDs of the original 20.
    for (int m = 0; m < 3; ++m)
        for (const auto* id : { "input", "output" })
            layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{parameterID(m, id), 1}, juce::StringArray{"4SX", "3DR", "40G"}[m] + " " + id + " gain", juce::NormalisableRange<float>{-24.0f, 12.0f, 0.1f}, 0.0f));
    return layout;
}
SPVinylLabProcessor::SPVinylLabProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "SPVinylLabState", makeParameters())
{
    modeValue = parameters.getRawParameterValue("mode");
    enabledValue = parameters.getRawParameterValue("enabled");
    for (int m = 0; m < 3; ++m)
        for (int p = 0; p < 6; ++p) values[m][p] = parameters.getRawParameterValue(parameterID(m, names[p]));
    for (int m = 0; m < 3; ++m) {
        gainValues[m][0] = parameters.getRawParameterValue(parameterID(m, "input"));
        gainValues[m][1] = parameters.getRawParameterValue(parameterID(m, "output"));
    }
}
bool SPVinylLabProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{ return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo() && l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
void SPVinylLabProcessor::prepareToPlay(double sr, int)
{
    noise.reset(); wow.prepare(sr); compressor.prepare(sr); filter.prepare(sr);
    const auto m = juce::jlimit(0, 2, juce::roundToInt(modeValue->load()));
    juce::SmoothedValue<float>* smooth[] { &noiseAmount, &wowAmount, &compression, &tone, &drive, &mix };
    for (int p = 0; p < 6; ++p) { smooth[p]->reset(sr, 0.03); smooth[p]->setCurrentAndTargetValue(values[m][p]->load()); }
    inputGain.reset(sr, 0.03); outputGain.reset(sr, 0.03);
    inputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(gainValues[m][0]->load()));
    outputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(gainValues[m][1]->load()));
    inputPeak.store(0); outputPeak.store(0); peak.store(0);
}
void SPVinylLabProcessor::processBlock(juce::AudioBuffer<float>& b, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    midi.clear();
    if (b.getNumChannels() != 2) { b.clear(); return; }
    const auto capturePeak = [](std::atomic<float>& target, float value) {
        auto previous = target.load(std::memory_order_relaxed);
        while (previous < value && !target.compare_exchange_weak(previous, value, std::memory_order_relaxed)) {}
    };
    if (b.getNumSamples() > 0)
        capturePeak(inputPeak, juce::jmax(b.getMagnitude(0, 0, b.getNumSamples()), b.getMagnitude(1, 0, b.getNumSamples())));
    const auto m = juce::jlimit(0, 2, juce::roundToInt(modeValue->load()));
    juce::SmoothedValue<float>* smooth[] { &noiseAmount, &wowAmount, &compression, &tone, &drive, &mix };
    for (int p = 0; p < 6; ++p) smooth[p]->setTargetValue(values[m][p]->load());
    inputGain.setTargetValue(juce::Decibels::decibelsToGain(gainValues[m][0]->load()));
    outputGain.setTargetValue(juce::Decibels::decibelsToGain(gainValues[m][1]->load()));
    if (enabledValue->load() >= 0.5f)
    {
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            const float n = noiseAmount.getNextValue(), w = wowAmount.getNextValue(), c = compression.getNextValue();
            const float t = tone.getNextValue(), d = drive.getNextValue(), wet = mix.getNextValue();
            const float in = inputGain.getNextValue(), out = outputGain.getNextValue();
            float l = b.getSample(0, i) * in, r = b.getSample(1, i) * in;
            const float dryL = l, dryR = r;
            wow.process(l, r, w);
            filter.process(l, r, t);
            compressor.process(l, r, c);
            l = saturation.process(l, d); r = saturation.process(r, d);
            noise.process(l, r, n);
            b.setSample(0, i, (dryL + wet * (l - dryL)) * out); b.setSample(1, i, (dryR + wet * (r - dryR)) * out);
        }
    }
    float level = 0;
    if (b.getNumSamples() > 0) level = juce::jmax(b.getMagnitude(0, 0, b.getNumSamples()), b.getMagnitude(1, 0, b.getNumSamples()));
    peak.store(level, std::memory_order_relaxed);
    capturePeak(outputPeak, level);
}
void SPVinylLabProcessor::getStateInformation(juce::MemoryBlock& dest)
{ auto state = parameters.copyState(); state.setProperty("schemaVersion", 2, nullptr); if (auto xml = state.createXml()) copyXmlToBinary(*xml, dest); }
void SPVinylLabProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size)) {
        if (!xml->hasTagName(parameters.state.getType().toString())) return;
        auto state = juce::ValueTree::fromXml(*xml);
        // Version 1 projects predate gain faders. Missing gains must restore to
        // unity even when an older project is loaded into an already-used instance.
        const auto current = parameters.copyState();
        for (int m = 0; m < 3; ++m) {
            for (const auto* name : {"input", "output"}) {
                const auto id = parameterID(m, name);
                if (!state.getChildWithProperty("id", id).isValid()) {
                    auto child = current.getChildWithProperty("id", id).createCopy();
                    child.setProperty("value", 0.0f, nullptr);
                    state.appendChild(child, nullptr);
                }
            }
        }
        parameters.replaceState(state);
    }
}
juce::AudioProcessorEditor* SPVinylLabProcessor::createEditor() { return new SPVinylLabEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SPVinylLabProcessor(); }
