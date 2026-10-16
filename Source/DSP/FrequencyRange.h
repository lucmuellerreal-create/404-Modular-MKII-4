#pragma once
class FrequencyRange {
public:
    void prepare(double sampleRate) noexcept { sr = sampleRate; left = right = 0.0f; }
    void process(float& l, float& r, float tone) noexcept;
private:
    double sr = 44100;
    float left = 0.0f, right = 0.0f;
};
