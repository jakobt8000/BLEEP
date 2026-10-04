#pragma once
#include "Biquad.h"
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace bleep
{
// Ring modulator: the classic sci-fi robot.
struct RingMod
{
    double sr = 44100.0, phase = 0.0;
    void prepare (double s) { sr = s; phase = 0.0; }
    inline float process (float x, float amount, float freq)
    {
        const float m = (float) std::sin (2.0 * M_PI * phase);
        phase += freq / sr;
        if (phase >= 1.0) phase -= 1.0;
        return x * (1.0f - amount) + x * m * amount;
    }
};

// Tuned feedback comb: "voice inside a tin can / helmet".
struct TinCan
{
    double sr = 44100.0;
    std::vector<float> buf;
    int pos = 0, mask = 0;
    float delay = 100.0f;
    Biquad damp;

    void prepare (double s)
    {
        sr = s;
        int size = 1;
        while (size < (int) (0.05 * sr)) size <<= 1;
        buf.assign ((size_t) size, 0.0f);
        mask = size - 1; pos = 0;
        delay = (float) (0.004 * sr);
        damp.setLowpass (sr, 5000.0, 0.6); damp.reset();
    }

    inline float process (float x, float amount, float tuneHz)
    {
        // resonate an octave above the sung note, kept inside 1..20 ms
        const float target = std::clamp ((float) sr / std::max (tuneHz * 2.0f, 1.0f), (float) (0.001 * sr), (float) (0.02 * sr));
        delay += 0.002f * (target - delay);

        const float rp = (float) pos - delay;
        const int i0 = (int) std::floor (rp);
        const float t = rp - (float) i0;
        const float d = buf[(size_t) (i0 & mask)] * (1.0f - t) + buf[(size_t) ((i0 + 1) & mask)] * t;

        const float fb = 0.55f + 0.37f * amount;
        const float y = x + fb * damp.process (d);
        buf[(size_t) pos] = y;
        pos = (pos + 1) & mask;

        return x * (1.0f - amount) + y * (1.0f - fb) * 2.8f * amount;
    }
};

// Walkie-talkie: narrow band, a bit of grit and hiss that follows the voice.
struct Radio
{
    double sr = 44100.0;
    Biquad hp[2], lp[2];
    float env = 0.0f, envCoef = 0.999f, lastAmt = -1.0f;
    uint32_t seed = 777u;

    void prepare (double s)
    {
        sr = s;
        for (auto* f : { &hp[0], &hp[1], &lp[0], &lp[1] }) f->reset();
        envCoef = (float) std::exp (-1.0 / (0.03 * sr));
        lastAmt = -1.0f;
    }

    inline float process (float x, float amount)
    {
        if (std::abs (amount - lastAmt) > 0.01f)
        {
            lastAmt = amount;
            const double lo = 250.0 + 550.0 * amount, hi = 3600.0 - 1700.0 * amount;
            hp[0].setHighpass (sr, lo, 0.7); hp[1].setHighpass (sr, lo, 0.7);
            lp[0].setLowpass (sr, hi, 0.9);  lp[1].setLowpass (sr, hi, 0.9);
        }
        env = envCoef * env + (1.0f - envCoef) * std::abs (x);

        seed = seed * 1664525u + 1013904223u;
        const float noise = (float) ((int32_t) seed) * (1.0f / 2147483648.0f);

        float y = lp[1].process (lp[0].process (hp[1].process (hp[0].process (x))));
        const float g = 1.0f + 4.0f * amount;
        y = std::tanh (y * g) / std::sqrt (g) * 1.4f + noise * env * 0.25f * amount;
        return x * (1.0f - amount) + y * amount;
    }
};
} // namespace bleep
