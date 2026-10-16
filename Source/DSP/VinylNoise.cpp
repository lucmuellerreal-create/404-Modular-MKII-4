#include "VinylNoise.h"
float VinylNoise::random() noexcept
{
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return static_cast<float>(state >> 8) / 8388608.0f - 1.0f;
}
void VinylNoise::process(float& l, float& r, float amount) noexcept
{
    const float a = random(), b = random();
    const float crackle = a > 0.9995f ? b * 0.12f : 0.0f;
    l += amount * (a * 0.006f + crackle);
    r += amount * (b * 0.006f + crackle);
}
