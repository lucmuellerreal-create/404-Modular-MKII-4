#pragma once
class VintageCompressor {
public:
    void prepare(double sampleRate) noexcept;
    void process(float& l, float& r, float amount) noexcept;
private:
    float envelope = 0.0f, attack = 0.0f, release = 0.0f;
};
