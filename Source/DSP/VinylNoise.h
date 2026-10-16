#pragma once
#include <cstdint>
class VinylNoise {
public:
    void reset() noexcept { state = 0x13579bdu; }
    void process(float& l, float& r, float amount) noexcept;
private:
    float random() noexcept;
    std::uint32_t state = 0x13579bdu;
};
