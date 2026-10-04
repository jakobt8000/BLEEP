#pragma once
#include "Biquad.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace bleep
{
// Streaming YIN pitch detector running on a decimated (~11 kHz) copy of the input.
class PitchDetector
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        decim = std::max (1, (int) std::lround (sr / 11025.0));
        dsr = sr / decim;
        minLag = std::max (2, (int) std::floor (dsr / 1000.0));
        maxLag = (int) std::ceil (dsr / 60.0);
        window = maxLag;
        buf.assign ((size_t) (window + maxLag), 0.0f);
        diff.assign ((size_t) maxLag + 2, 0.0f);
        hop = std::max (16, (int) (dsr * 0.004));
        for (auto& f : lp) { f.setLowpass (sr, 1200.0, 0.707); f.reset(); }
        hp.setHighpass (sr, 50.0, 0.707); hp.reset();
        decimCount = 0; hopCount = 0; filled = 0;
        energy = 0.0f;
        voiced = false; periodSamples = (float) (sr / 150.0); confidence = 0.0f;
    }

    // Feed one input sample (at the host rate).
    inline void push (float x)
    {
        x = hp.process (x);
        float y = lp[1].process (lp[0].process (x));
        if (++decimCount < decim) return;
        decimCount = 0;

        // shift-register style ring (small buffer, cheap enough)
        std::move (buf.begin() + 1, buf.end(), buf.begin());
        buf.back() = y;
        if (filled < (int) buf.size()) ++filled;

        if (++hopCount >= hop)
        {
            hopCount = 0;
            if (filled >= (int) buf.size()) analyse();
        }
    }

    bool isVoiced() const        { return voiced; }
    float getPeriod() const      { return periodSamples; }   // in host samples
    float getFrequency() const   { return (float) (sr / periodSamples); }
    float getConfidence() const  { return confidence; }

private:
    void analyse()
    {
        const int W = window;
        const float* x = buf.data();

        float e = 0.0f;
        for (int j = 0; j < W + maxLag; ++j) e += x[j] * x[j];
        e /= (float) (W + maxLag);
        energy = e;

        if (e < 1.0e-6f) { voiced = false; confidence = 0.0f; return; } // ~ -60 dBFS gate

        diff[0] = 1.0f;
        float running = 0.0f;
        for (int tau = 1; tau <= maxLag; ++tau)
        {
            float d = 0.0f;
            for (int j = 0; j < W; ++j)
            {
                const float t = x[j] - x[j + tau];
                d += t * t;
            }
            running += d;
            diff[(size_t) tau] = running > 0.0f ? d * (float) tau / running : 1.0f;
        }

        int best = -1;
        const float threshold = 0.18f;
        for (int tau = minLag; tau < maxLag; ++tau)
        {
            if (diff[(size_t) tau] < threshold)
            {
                while (tau + 1 < maxLag && diff[(size_t) tau + 1] < diff[(size_t) tau]) ++tau;
                best = tau;
                break;
            }
        }

        if (best < 0) { voiced = false; confidence = 0.0f; return; }

        // parabolic interpolation
        float t = (float) best;
        if (best > 1 && best < maxLag)
        {
            const float a = diff[(size_t) best - 1], b = diff[(size_t) best], c = diff[(size_t) best + 1];
            const float den = a - 2.0f * b + c;
            if (std::abs (den) > 1.0e-9f) t += 0.5f * (a - c) / den;
        }

        const float newPeriod = t * (float) decim;
        confidence = 1.0f - diff[(size_t) best];

        if (! voiced || std::abs (std::log (newPeriod / periodSamples)) > 0.15f)
            periodSamples = newPeriod; // jump on onset / big moves
        else
            periodSamples = std::exp (std::log (periodSamples) + 0.5f * (std::log (newPeriod) - std::log (periodSamples)));

        voiced = true;
    }

    double sr = 44100.0, dsr = 11025.0;
    int decim = 4, minLag = 11, maxLag = 184, window = 184, hop = 44;
    int decimCount = 0, hopCount = 0, filled = 0;
    std::vector<float> buf, diff;
    Biquad lp[2], hp;
    float energy = 0.0f;
    bool voiced = false;
    float periodSamples = 300.0f, confidence = 0.0f;
};
} // namespace bleep
