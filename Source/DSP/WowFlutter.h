#pragma once
#include <array>
#include <vector>
class WowFlutter {
public:
    void prepare(double sampleRate);
    void process(float& l, float& r, float amount) noexcept;
private:
    std::array<std::vector<float>, 2> ring;
    int write = 0;
    double sr = 44100.0, phase = 0.0;
};
