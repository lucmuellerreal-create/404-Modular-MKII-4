#include "VintageCompressor.h"
#include <cmath>
#include <algorithm>
void VintageCompressor::prepare(double sr) noexcept
{ envelope = 0; attack = static_cast<float>(std::exp(-1.0 / (sr * 0.008))); release = static_cast<float>(std::exp(-1.0 / (sr * 0.15))); }
void VintageCompressor::process(float& l, float& r, float amount) noexcept
{
    const float level = std::max(std::abs(l), std::abs(r));
    const float coefficient = level > envelope ? attack : release;
    envelope = coefficient * envelope + (1.0f - coefficient) * level;
    const float threshold = 0.75f - 0.65f * amount;
    const float gain = envelope > threshold ? std::pow(threshold / envelope, amount * 0.75f) : 1.0f;
    l *= gain; r *= gain;
}
