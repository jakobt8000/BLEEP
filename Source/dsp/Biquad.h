#pragma once
#include <cmath>

namespace bleep
{
// Plain RBJ biquad (transposed direct form II).
struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1 = 0, z2 = 0;

    void reset() { z1 = z2 = 0; }

    void setLowpass (double sr, double freq, double q)
    {
        const double w = 2.0 * M_PI * freq / sr, c = std::cos (w), a = std::sin (w) / (2.0 * q);
        const double n = 1.0 / (1.0 + a);
        b0 = (float) ((1.0 - c) * 0.5 * n); b1 = (float) ((1.0 - c) * n); b2 = b0;
        a1 = (float) (-2.0 * c * n);        a2 = (float) ((1.0 - a) * n);
    }

    // Constant 0 dB peak gain bandpass.
    void setBandpass (double sr, double freq, double q)
    {
        const double w = 2.0 * M_PI * freq / sr, c = std::cos (w), a = std::sin (w) / (2.0 * q);
        const double n = 1.0 / (1.0 + a);
        b0 = (float) (a * n); b1 = 0.0f; b2 = (float) (-a * n);
        a1 = (float) (-2.0 * c * n); a2 = (float) ((1.0 - a) * n);
    }

    void setHighpass (double sr, double freq, double q)
    {
        const double w = 2.0 * M_PI * freq / sr, c = std::cos (w), a = std::sin (w) / (2.0 * q);
        const double n = 1.0 / (1.0 + a);
        b0 = (float) ((1.0 + c) * 0.5 * n); b1 = (float) (-(1.0 + c) * n); b2 = b0;
        a1 = (float) (-2.0 * c * n);        a2 = (float) ((1.0 - a) * n);
    }

    inline float process (float x)
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
};
} // namespace bleep
