#include "FrequencyRange.h"
#include <cmath>
void FrequencyRange::process(float& l, float& r, float tone) noexcept
{
    const double cutoff = 1500.0 + 16500.0 * tone * tone;
    const float a = static_cast<float>(1.0 - std::exp(-6.283185307179586 * cutoff / sr));
    left += a * (l - left); right += a * (r - right);
    l = left; r = right;
}
