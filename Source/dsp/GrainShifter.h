#pragma once
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace bleep
{
// Pitch-synchronous overlap-add (TD-PSOLA style) pitch + formant shifter.
//
// Grains of roughly two input periods are cut out around analysis marks that
// walk through the input one detected period at a time, and are laid down at
// synthesis marks spaced by the *target* period. Resampling each grain by the
// formant ratio moves the spectral envelope independently of the pitch.
//
// Output is delayed by getLatency() samples relative to the input.
class GrainShifter
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        maxSpan = (int) std::ceil (0.0125 * sr);                  // max input half-span of a grain
        const int maxHalfPeriod = (int) std::ceil (sr / 60.0 * 0.5);
        latency = 2 * maxSpan + maxHalfPeriod + 4;

        int size = 1;
        while (size < 4 * (latency + 4 * maxSpan)) size <<= 1;
        mask = size - 1;
        inBuf.assign ((size_t) size, 0.0f);
        acc.assign ((size_t) size, 0.0f);
        frozenBuf.assign ((size_t) (4 * maxSpan + 8), 0.0f);
        frozen = false;

        n = 0;
        nextMark = (double) latency;
        anaMark = 0.0;
        wasVoiced = false;
        unvoicedPeriod = (float) (0.006 * sr);
    }

    int getLatency() const { return latency; }

    // inPeriod / outPeriod in samples; formantRatio > 1 = smaller/brighter "head".
    // Keep the input history running without producing grains (cheap bypass).
    inline void idle (float x)
    {
        inBuf[(size_t) (n & mask)] = x;
        acc[(size_t) (n & mask)] = 0.0f;
        ++n;
        nextMark = (double) n;
        frozen = false;
    }

    // freeze = loop the current grain forever (a sustained drone of the voice)
    inline float process (float x, bool voiced, float inPeriod, float outPeriod, float formantRatio, bool freeze = false)
    {
        inBuf[(size_t) (n & mask)] = x;

        if (freeze && ! frozen)
        {
            const int64_t start = n - (int64_t) (4 * maxSpan) - 4;
            for (size_t k = 0; k < frozenBuf.size(); ++k)
                frozenBuf[k] = inBuf[(size_t) ((start + (int64_t) k) & mask)];
        }
        if (! freeze && frozen) wasVoiced = ! voiced;   // force a fresh analysis mark
        frozen = freeze;

        if (! voiced)
        {
            inPeriod = outPeriod = unvoicedPeriod;
        }

        const float f = std::clamp (formantRatio, 0.5f, 2.0f);
        inPeriod  = std::clamp (inPeriod, 16.0f, (float) (sr / 50.0));
        outPeriod = std::clamp (outPeriod, 16.0f, (float) (sr / 40.0));

        // Half-length of the grain in output samples. Voiced: one input period
        // (classic PSOLA, gaps between grains are what lower the pitch).
        // Unvoiced: make sure grains always overlap so noise stays smooth.
        float H = voiced ? inPeriod / f : std::max (outPeriod, inPeriod / f);
        H = std::min (H, (float) maxSpan);
        H = std::min (H, (float) maxSpan / f);

        // place grains just in time
        while (nextMark - H <= (double) n)
        {
            const double target = nextMark - latency; // matching input time

            if (voiced != wasVoiced || ! voiced || std::abs (anaMark - target) > 2.0 * inPeriod)
                anaMark = target;
            else
                while (anaMark + 0.5 * inPeriod < target) anaMark += inPeriod;

            wasVoiced = voiced;

            // never read samples we don't have yet
            while (anaMark + H * f > (double) n - 2.0) anaMark -= inPeriod;

            // keep the average overlap-sum at unity when grains pile up
            if (frozen)
            {
                double c = 2.0 * maxSpan + 2.0;
                if (! voiced) c += (rng() * 2.0 - 1.0) * 0.9 * maxSpan;   // shuffle noise grains
                addGrain (nextMark, c, H, f, std::min (1.0f, outPeriod / H), true);
            }
            else
                addGrain (nextMark, anaMark, H, f, std::min (1.0f, outPeriod / H), false);
            nextMark += outPeriod;
        }

        const size_t idx = (size_t) (n & mask);
        const float y = acc[idx];
        acc[idx] = 0.0f;
        ++n;
        return y;
    }

private:
    inline double rng()
    {
        seed = seed * 1664525u + 1013904223u;
        return (double) (seed >> 8) / 16777216.0;
    }

    inline float readFrozen (double pos) const
    {
        const double fl = std::floor (pos);
        const int i = std::clamp ((int) fl, 0, (int) frozenBuf.size() - 2);
        const float t = (float) (pos - fl);
        return frozenBuf[(size_t) i] + t * (frozenBuf[(size_t) i + 1] - frozenBuf[(size_t) i]);
    }

    inline float readAt (double pos) const
    {
        const double fl = std::floor (pos);
        const int64_t i = (int64_t) fl;
        const float t = (float) (pos - fl);
        const float xm1 = inBuf[(size_t) ((i - 1) & mask)];
        const float x0  = inBuf[(size_t) (i & mask)];
        const float x1  = inBuf[(size_t) ((i + 1) & mask)];
        const float x2  = inBuf[(size_t) ((i + 2) & mask)];
        // 4-point Hermite
        const float c1 = 0.5f * (x1 - xm1);
        const float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
        const float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
        return ((c3 * t + c2) * t + c1) * t + x0;
    }

    void addGrain (double outCentre, double inCentre, float H, float f, float gain, bool fromFrozen)
    {
        const int64_t c = (int64_t) std::llround (outCentre);
        const double frac = outCentre - (double) c;
        const int h = (int) std::ceil (H);
        const float invH = 1.0f / H;
        for (int i = -h; i <= h; ++i)
        {
            const float rel = ((float) i - (float) frac) * invH;
            if (rel <= -1.0f || rel >= 1.0f) continue;
            const float w = gain * (0.5f + 0.5f * std::cos ((float) M_PI * rel));
            const int64_t o = c + i;
            if (o < n) continue; // already played
            const double pos = inCentre + ((double) i - frac) * f;
            const float s = fromFrozen ? readFrozen (pos) : readAt (pos);
            acc[(size_t) (o & mask)] += w * s;
        }
    }

    double sr = 44100.0;
    int maxSpan = 512, latency = 1500, mask = 0;
    std::vector<float> inBuf, acc, frozenBuf;
    bool frozen = false;
    uint32_t seed = 12345u;
    int64_t n = 0;
    double nextMark = 0.0, anaMark = 0.0;
    bool wasVoiced = false;
    float unvoicedPeriod = 256.0f;
};
} // namespace bleep
