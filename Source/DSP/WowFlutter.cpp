#include "WowFlutter.h"
#include <cmath>
#include <algorithm>
void WowFlutter::prepare(double sampleRate)
{
    sr = sampleRate; phase = 0; write = 0;
    for (auto& channel : ring) channel.assign(static_cast<std::size_t>(sr * 0.02) + 8, 0.0f);
}
void WowFlutter::process(float& l, float& r, float amount) noexcept
{
    const int size = static_cast<int>(ring[0].size());
    if (size == 0) return;
    const float input[] {l, r};
    const double modulation = 0.5 + 0.38 * std::sin(phase) + 0.12 * std::sin(phase * 13.7);
    const double delay = amount * sr * 0.006 * modulation;
    double read = static_cast<double>(write) - delay;
    if (read < 0) read += size;
    const int a = static_cast<int>(read), b = (a + 1) % size;
    const float fraction = static_cast<float>(read - a);
    float result[2];
    for (int ch = 0; ch < 2; ++ch) {
        ring[ch][static_cast<std::size_t>(write)] = input[ch];
        result[ch] = ring[ch][static_cast<std::size_t>(a)] * (1.0f - fraction) + ring[ch][static_cast<std::size_t>(b)] * fraction;
    }
    l = result[0]; r = result[1]; write = (write + 1) % size;
    phase += 6.283185307179586 * 0.55 / sr;
    // Common period keeps both oscillators continuous across wrapping.
    if (phase >= 62.83185307179586) phase -= 62.83185307179586;
}
