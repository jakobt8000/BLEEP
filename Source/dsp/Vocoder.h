#pragma once
#include "Biquad.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace bleep
{
// 20-band channel vocoder with a built-in carrier oscillator.
// The voice (modulator) shapes the carrier band by band. Synthesis bands are
// shifted by the formant ratio so the FORMANT knob also works on the vocoder.
class Vocoder
{
public:
    static constexpr int numBands = 20;
    enum Wave { Saw = 0, Square, Buzz, Noise };

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        for (int b = 0; b < numBands; ++b)
        {
            const double t = (double) b / (numBands - 1);
            centre[(size_t) b] = 100.0 * std::pow (8000.0 / 100.0, t);
            ana[(size_t) b][0].reset(); ana[(size_t) b][1].reset();
            syn[(size_t) b][0].reset(); syn[(size_t) b][1].reset();
            env[(size_t) b] = 0.0f;
        }
        attack  = (float) std::exp (-1.0 / (0.002 * sr));
        release = (float) std::exp (-1.0 / (0.025 * sr));
        rmsCoef = (float) std::exp (-1.0 / (0.150 * sr));
        lastFormant = -1.0f;
        updateFilters (1.0f);
        phase = 0.0; rmsIn = rmsOut = 0.0f; gain = 1.0f;
    }

    inline float process (float voice, float carrierHz, float formantRatio, int wave, float unvoiced)
    {
        if (++updateCounter >= 32)
        {
            updateCounter = 0;
            if (std::abs (formantRatio - lastFormant) > 0.002f) updateFilters (formantRatio);
        }

        // --- carrier
        const double dt = std::clamp ((double) carrierHz, 20.0, 2000.0) / sr;
        float osc = 0.0f;
        switch (wave)
        {
            case Saw:    osc = (float) (2.0 * phase - 1.0) - polyBlep (phase, dt); break;
            case Square: osc = (phase < 0.5 ? 1.0f : -1.0f) + polyBlep (phase, dt) - polyBlep (std::fmod (phase + 0.5, 1.0), dt); break;
            case Buzz:   osc = (phase < 0.12 ? 1.0f : -0.14f) + polyBlep (phase, dt) * 0.57f - polyBlep (std::fmod (phase + 0.88, 1.0), dt) * 0.57f; break;
            default:     osc = 0.0f; break;
        }
        phase += dt;
        if (phase >= 1.0) phase -= 1.0;

        const float nz = noise();
        const float uv = (wave == Noise) ? 1.0f : std::clamp (unvoiced, 0.0f, 1.0f);
        const float carrier = osc * (1.0f - uv) + nz * (uv * 0.8f + 0.04f);

        // --- bands
        float out = 0.0f;
        for (int b = 0; b < numBands; ++b)
        {
            const size_t i = (size_t) b;
            const float m = std::abs (ana[i][1].process (ana[i][0].process (voice)));
            float& e = env[i];
            e = m > e ? attack * e + (1.0f - attack) * m : release * e + (1.0f - release) * m;
            out += syn[i][1].process (syn[i][0].process (carrier)) * e;
        }

        // --- automatic level match so the vocoder sits at the voice's level
        rmsIn  = rmsCoef * rmsIn  + (1.0f - rmsCoef) * voice * voice;
        rmsOut = rmsCoef * rmsOut + (1.0f - rmsCoef) * out * out;
        if (rmsOut > 1.0e-10f && rmsIn > 1.0e-9f)
        {
            const float target = std::clamp (std::sqrt (rmsIn / rmsOut), 0.5f, 400.0f);
            gain += 0.0005f * (target - gain);
        }
        return out * gain;
    }

private:
    void updateFilters (float formantRatio)
    {
        lastFormant = formantRatio;
        const double nyq = sr * 0.45;
        for (int b = 0; b < numBands; ++b)
        {
            const size_t i = (size_t) b;
            const double q = 6.0;
            ana[i][0].setBandpass (sr, std::min (centre[i], nyq), q);
            ana[i][1].setBandpass (sr, std::min (centre[i], nyq), q);
            const double s = std::min (centre[i] * formantRatio, nyq);
            syn[i][0].setBandpass (sr, s, q);
            syn[i][1].setBandpass (sr, s, q);
        }
    }

    static inline float polyBlep (double t, double dt)
    {
        if (t < dt)       { t /= dt;             return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt;  return (float) (t * t + t + t + 1.0); }
        return 0.0f;
    }

    inline float noise()
    {
        seed = seed * 1664525u + 1013904223u;
        return (float) ((int32_t) seed) * (1.0f / 2147483648.0f);
    }

    double sr = 44100.0, phase = 0.0;
    std::array<double, numBands> centre {};
    std::array<std::array<Biquad, 2>, numBands> ana {}, syn {};
    std::array<float, numBands> env {};
    float attack = 0.9f, release = 0.99f, rmsCoef = 0.999f;
    float rmsIn = 0.0f, rmsOut = 0.0f, gain = 1.0f, lastFormant = -1.0f;
    uint32_t seed = 22222u;
    int updateCounter = 0;
};
} // namespace bleep
