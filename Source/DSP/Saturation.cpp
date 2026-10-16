#include "Saturation.h"
#include <cmath>
float Saturation::process(float input, float amount) const noexcept
{
    const float gain = 1.0f + amount * 6.0f;
    return input + amount * (std::tanh(input * gain) / std::tanh(gain) - input);
}
