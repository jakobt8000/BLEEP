#include "PluginProcessor.h"
#include "PluginEditor.h"

BleepProcessor::BleepProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "BLEEP", createLayout())
{
    pPitch   = apvts.getRawParameterValue (ParamID::pitch);
    pFormant = apvts.getRawParameterValue (ParamID::formant);
    pRobot   = apvts.getRawParameterValue (ParamID::robot);
    pNote    = apvts.getRawParameterValue (ParamID::note);
    pOctave  = apvts.getRawParameterValue (ParamID::octave);
    pCarrier = apvts.getRawParameterValue (ParamID::carrier);
    pWave    = apvts.getRawParameterValue (ParamID::wave);
    pCrush   = apvts.getRawParameterValue (ParamID::crush);
    pDrive   = apvts.getRawParameterValue (ParamID::drive);
    pMix     = apvts.getRawParameterValue (ParamID::mix);
    pOutput  = apvts.getRawParameterValue (ParamID::output);
    pSnap    = apvts.getRawParameterValue (ParamID::snap);
    pScale   = apvts.getRawParameterValue (ParamID::scale);
    pHarmony = apvts.getRawParameterValue (ParamID::harmony);
    pVoices  = apvts.getRawParameterValue (ParamID::voices);
    pRing    = apvts.getRawParameterValue (ParamID::ring);
    pRingHz  = apvts.getRawParameterValue (ParamID::ringHz);
    pTin     = apvts.getRawParameterValue (ParamID::tin);
    pRadio   = apvts.getRawParameterValue (ParamID::radio);
    pWobble  = apvts.getRawParameterValue (ParamID::wobble);
    pRate    = apvts.getRawParameterValue (ParamID::rate);
    pFreeze  = apvts.getRawParameterValue (ParamID::freeze);
    pKey     = apvts.getRawParameterValue (ParamID::key);
    pRobotOn = apvts.getRawParameterValue (ParamID::robotOn);
    pSnapOn  = apvts.getRawParameterValue (ParamID::snapOn);
    pHarmOn  = apvts.getRawParameterValue (ParamID::harmOn);
    pVocOn   = apvts.getRawParameterValue (ParamID::vocOn);
    pRingOn  = apvts.getRawParameterValue (ParamID::ringOn);
    pTinOn   = apvts.getRawParameterValue (ParamID::tinOn);
    pRadioOn = apvts.getRawParameterValue (ParamID::radioOn);
    pWobOn   = apvts.getRawParameterValue (ParamID::wobOn);
    pDirtOn  = apvts.getRawParameterValue (ParamID::dirtOn);
}

juce::AudioProcessorValueTreeState::ParameterLayout BleepProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto pct = AudioParameterFloatAttributes().withLabel ("%")
                 .withStringFromValueFunction ([] (float v, int) { return String (juce::roundToInt (v)) + "%"; });
    auto st  = AudioParameterFloatAttributes().withLabel ("st")
                 .withStringFromValueFunction ([] (float v, int) { return (v > 0.05f ? "+" : "") + String (v, 1) + " st"; });

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::pitch, 1 },   "Pitch",   NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f, st));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::formant, 1 }, "Formant", NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f, st));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::robot, 1 },   "Robot",   NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::note, 1 },   "Note",
                    StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
    p.push_back (std::make_unique<AudioParameterInt>   (ParameterID { ParamID::octave, 1 },  "Octave",  1, 5, 3));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::carrier, 1 }, "Vocoder", NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::wave, 1 },   "Wave",
                    StringArray { "SAW", "SQR", "BUZZ", "AIR" }, 0));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::crush, 1 },   "Crush",   NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::drive, 1 },   "Drive",   NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::mix, 1 },     "Mix",     NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, pct));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::output, 1 },  "Output",  NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f,
                    AudioParameterFloatAttributes().withLabel ("dB")
                        .withStringFromValueFunction ([] (float v, int) { return (v > 0.05f ? "+" : "") + String (v, 1) + " dB"; })));

    auto hz = AudioParameterFloatAttributes().withLabel ("Hz")
                .withStringFromValueFunction ([] (float v, int) { return v < 10.0f ? String (v, 1) + " Hz" : String (juce::roundToInt (v)) + " Hz"; });
    auto logRange = [] (float lo, float hi)
    {
        return NormalisableRange<float> (lo, hi,
            [] (float a, float b, float t) { return a * std::pow (b / a, t); },
            [] (float a, float b, float v) { return std::log (v / a) / std::log (b / a); });
    };

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::snap, 1 },    "Snap",    NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::scale, 1 },  "Scale",
                    StringArray { "Chromatic", "Major", "Minor", "Penta" }, 1));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::harmony, 1 }, "Harmony", NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::voices, 1 }, "Voices",
                    StringArray { "-12", "+7", "+4 +7", "-12 +12" }, 0));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::ring, 1 },    "Ring",    NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::ringHz, 1 },  "Ring Freq", logRange (10.0f, 2000.0f), 30.0f, hz));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::tin, 1 },     "Tin",     NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::radio, 1 },   "Radio",   NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::wobble, 1 },  "Wobble",  NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::rate, 1 },    "Wobble Rate", logRange (0.1f, 12.0f), 2.0f, hz));
    p.push_back (std::make_unique<AudioParameterBool>  (ParameterID { ParamID::freeze, 1 },  "Freeze",  false));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::key, 1 },    "Key",
                    StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
    for (auto [id, name] : { std::pair { ParamID::robotOn, "Robot On" }, { ParamID::snapOn, "Snap On" }, { ParamID::harmOn, "Harmony On" },
                             { ParamID::vocOn, "Vocoder On" }, { ParamID::ringOn, "Ring On" }, { ParamID::tinOn, "Tin On" },
                             { ParamID::radioOn, "Radio On" }, { ParamID::wobOn, "Wobble On" }, { ParamID::dirtOn, "Dirt On" } })
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, false));

    return { p.begin(), p.end() };
}

bool BleepProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet(), out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo()) return false;
    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo()) return false;
    return in.size() <= out.size();
}

void BleepProcessor::prepareToPlay (double sampleRate, int)
{
    detector.prepare (sampleRate);
    shifter.prepare (sampleRate);
    vocoder.prepare (sampleRate);
    harm1.prepare (sampleRate);
    harm2.prepare (sampleRate);
    ringMod.prepare (sampleRate);
    tinCan.prepare (sampleRate);
    radioFx.prepare (sampleRate);
    snappedMidi = 50.0f; wobblePhase = 0.0; prevFreeze = false;

    latency = shifter.getLatency();
    setLatencySamples (latency);

    delaySize = latency + 1;
    dryDelay.setSize (2, delaySize);
    dryDelay.clear();
    monoDelay.assign ((size_t) delaySize, 0.0f);
    hzDelay.assign ((size_t) delaySize, 147.0f);
    uvDelay.assign ((size_t) delaySize, 1.0f);
    delayPos = 0;

    glideCoef = (float) std::exp (-1.0 / (0.012 * sampleRate));
    uvCoef    = (float) std::exp (-1.0 / (0.006 * sampleRate));
    lastLogIn = 7.2f; carrierHzSmoothed = 147.0f; unvoicedSmoothed = 1.0f;
    crushHold = 0.0f; crushCounter = 0; levelFollow = 0.0f;

    for (auto* s : { &mixS, &outS, &carrierS, &driveS, &harmS, &ringS, &tinS, &radioS, &robotS, &wobS }) s->reset (sampleRate, 0.03);
    robotS.setCurrentAndTargetValue (0.0f);
    wobS.setCurrentAndTargetValue (0.0f);
    harmS.setCurrentAndTargetValue (pHarmony->load() * 0.01f);
    ringS.setCurrentAndTargetValue (pRing->load() * 0.01f);
    tinS.setCurrentAndTargetValue (pTin->load() * 0.01f);
    radioS.setCurrentAndTargetValue (pRadio->load() * 0.01f);
    mixS.setCurrentAndTargetValue (pMix->load() * 0.01f);
    outS.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    carrierS.setCurrentAndTargetValue (pCarrier->load() * 0.01f);
    driveS.setCurrentAndTargetValue (pDrive->load() * 0.01f);
}

void BleepProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numIn  = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    const double sr = getSampleRate();

    for (int ch = numIn; ch < numOut; ++ch) buffer.clear (ch, 0, numSamples);

    const float pitchSt   = pPitch->load();
    const float formantR  = std::pow (2.0f, pFormant->load() / 12.0f);
    auto on = [] (std::atomic<float>* p) { return p->load() > 0.5f ? 1.0f : 0.0f; };
    robotS.setTargetValue (pRobot->load() * 0.01f * on (pRobotOn));
    wobS.setTargetValue (pWobble->load() * 0.01f * on (pWobOn));
    const int   midiNote  = 12 * ((int) pOctave->load() + 1) + (int) pNote->load();
    const float noteLog   = std::log2 (440.0f) + (float) (midiNote - 69) / 12.0f;
    const int   wave      = (int) pWave->load();
    const float crush     = pCrush->load() * 0.01f * on (pDirtOn);

    mixS.setTargetValue (pMix->load() * 0.01f);
    outS.setTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    carrierS.setTargetValue (pCarrier->load() * 0.01f * on (pVocOn));
    driveS.setTargetValue (pDrive->load() * 0.01f * on (pDirtOn));
    harmS.setTargetValue (pHarmony->load() * 0.01f * on (pHarmOn));
    ringS.setTargetValue (pRing->load() * 0.01f * on (pRingOn));
    tinS.setTargetValue (pTin->load() * 0.01f * on (pTinOn));
    radioS.setTargetValue (pRadio->load() * 0.01f * on (pRadioOn));

    // --- SNAP: retune speed and scale
    const float snap = pSnap->load() * 0.01f * on (pSnapOn);
    const float retuneTime = (1.0f - snap) * (1.0f - snap) * 0.3f;
    const float snapCoef = retuneTime < 1.0e-4f ? 0.0f : (float) std::exp (-1.0 / (retuneTime * sr));
    static const int scaleMasks[4] = { 0xFFF, 0xAB5, 0x5AD, 0x295 };   // chromatic, major, minor, major pentatonic
    const int scaleMask = scaleMasks[juce::jlimit (0, 3, (int) pScale->load())];
    const int root = (int) pKey->load();
    auto quantise = [&] (float midi)
    {
        const int base = (int) std::floor (midi);
        int best = base; float bestD = 1.0e9f;
        for (int k = -7; k <= 8; ++k)
        {
            const int m = base + k;
            if ((scaleMask >> (((m - root) % 12 + 12) % 12)) & 1)
            {
                const float d = std::abs ((float) m - midi);
                if (d < bestD) { bestD = d; best = m; }
            }
        }
        return (float) best;
    };

    // --- HARMONY intervals
    static const float voiceSets[4][2] = { { -12.0f, 0.0f }, { 7.0f, 0.0f }, { 4.0f, 7.0f }, { -12.0f, 12.0f } };
    const int voiceSet = juce::jlimit (0, 3, (int) pVoices->load());
    const float h1Ratio = std::exp2 (-voiceSets[voiceSet][0] / 12.0f);   // period ratio
    const float h2Ratio = std::exp2 (-voiceSets[voiceSet][1] / 12.0f);
    const bool twoVoices = voiceSets[voiceSet][1] != 0.0f;

    // --- WOBBLE / RING / FREEZE
    const double wobbleInc = pRate->load() / sr;
    const float ringHz = pRingHz->load();
    const bool freeze = pFreeze->load() > 0.5f;
    if (freeze && ! prevFreeze)
    {
        frozenVoiced = detector.isVoiced();
        frozenPeriod = detector.getPeriod();
        frozenLogIn  = lastLogIn;
    }
    prevFreeze = freeze;

    const float bits  = 16.0f - crush * 13.0f;
    const float quant = std::pow (2.0f, bits - 1.0f);
    const int holdLen = 1 + (int) (crush * crush * 15.0f * (float) (sr / 48000.0));

    float lastInHz = 0.0f, lastOutHz = 0.0f, peak = 0.0f;
    const float inScale = 1.0f / (float) juce::jmax (1, numIn);

    for (int i = 0; i < numSamples; ++i)
    {
        float mono = 0.0f;
        for (int ch = 0; ch < numIn; ++ch) mono += buffer.getReadPointer (ch)[i];
        mono *= inScale;

        detector.push (mono);
        const bool voiced = freeze ? frozenVoiced : detector.isVoiced();
        const float inPeriod = freeze ? frozenPeriod : detector.getPeriod();
        if (voiced && ! freeze) lastLogIn = std::log2 ((float) sr / inPeriod);
        float inLog = freeze ? frozenLogIn : lastLogIn;

        // SNAP: glide toward the nearest scale note
        const float inMidi = 69.0f + 12.0f * (inLog - std::log2 (440.0f));
        if (snap > 0.0005f)
        {
            const float target = quantise (inMidi);
            if (std::abs (target - snappedMidi) > 12.0f) snappedMidi = target;
            snappedMidi = snapCoef * snappedMidi + (1.0f - snapCoef) * target;
            const float amt = juce::jmin (1.0f, snap * 4.0f);   // fade in over the first 25%
            inLog += amt * (snappedMidi - inMidi) / 12.0f;
        }
        else snappedMidi = inMidi;

        // WOBBLE: slow talkbox-style movement of formant and a little pitch
        const float robot  = robotS.getNextValue();
        const float wobble = wobS.getNextValue();
        const float lfoA = (float) std::sin (2.0 * M_PI * wobblePhase);
        const float lfoB = (float) std::sin (2.0 * M_PI * (wobblePhase + 0.25));
        wobblePhase += wobbleInc;
        if (wobblePhase >= 1.0) wobblePhase -= 1.0;
        const float formant = formantR * std::exp2 (wobble * 6.0f * lfoA / 12.0f);

        const float targetLog = (1.0f - robot) * inLog + robot * noteLog + (pitchSt + wobble * 0.6f * lfoB) / 12.0f;
        const float targetHz  = std::exp2 (targetLog);
        const float outPeriod = (float) sr / targetHz;

        float shifted = shifter.process (mono, voiced, inPeriod, outPeriod, formant, freeze);

        // HARMONY: extra shifted voices
        const float harm = harmS.getNextValue();
        if (harm > 0.0005f)
        {
            float h = harm1.process (mono, voiced, inPeriod, outPeriod * h1Ratio, formant, freeze);
            if (twoVoices) h += harm2.process (mono, voiced, inPeriod, outPeriod * h2Ratio, formant, freeze);
            else           harm2.idle (mono);
            shifted = shifted * (1.0f - 0.3f * harm) + harm * (twoVoices ? 0.45f : 0.6f) * h;
        }
        else { harm1.idle (mono); harm2.idle (mono); }

        // aligned (delayed) signals for the vocoder and dry path
        const float monoD = monoDelay[(size_t) delayPos];
        const float hzD   = hzDelay[(size_t) delayPos];
        const float uvD   = uvDelay[(size_t) delayPos];
        monoDelay[(size_t) delayPos] = mono;
        hzDelay[(size_t) delayPos]   = targetHz;
        uvDelay[(size_t) delayPos]   = voiced ? 0.0f : 1.0f;

        carrierHzSmoothed = glideCoef * carrierHzSmoothed + (1.0f - glideCoef) * hzD;
        unvoicedSmoothed  = uvCoef * unvoicedSmoothed + (1.0f - uvCoef) * uvD;

        const float carrierAmt = carrierS.getNextValue();
        float wet = shifted;
        if (carrierAmt > 0.0005f)
        {
            const float voc = vocoder.process (monoD, carrierHzSmoothed, formant, wave, unvoicedSmoothed);
            wet = shifted * (1.0f - carrierAmt) + voc * carrierAmt;
        }

        // COLOR
        const float ringAmt = ringS.getNextValue();
        if (ringAmt > 0.0005f) wet = ringMod.process (wet, ringAmt, ringHz);
        const float tinAmt = tinS.getNextValue();
        if (tinAmt > 0.0005f) wet = tinCan.process (wet, tinAmt, carrierHzSmoothed);
        const float radioAmt = radioS.getNextValue();
        if (radioAmt > 0.0005f) wet = radioFx.process (wet, radioAmt);

        const float drive = driveS.getNextValue();
        if (drive > 0.0005f)
        {
            const float g = 1.0f + drive * 20.0f;
            wet = std::tanh (wet * g) / std::sqrt (g);
        }

        if (crush > 0.0005f)
        {
            if (crushCounter <= 0)
            {
                crushHold = std::round (wet * quant) / quant;
                crushCounter = holdLen;
            }
            --crushCounter;
            wet = crushHold;
        }

        const float mix = mixS.getNextValue();
        const float outGain = outS.getNextValue();

        for (int ch = 0; ch < numOut; ++ch)
        {
            const int srcCh = juce::jmin (ch, numIn - 1);
            float* dd = dryDelay.getWritePointer (juce::jmin (ch, 1));
            const float dryD = dd[delayPos];
            if (srcCh >= 0 && ch < 2) dd[delayPos] = buffer.getReadPointer (srcCh)[i];
            float y = (dryD * (1.0f - mix) + wet * mix) * outGain;
            // soft safety limiter: transparent below -1 dBFS
            constexpr float knee = 0.891f;
            if (std::abs (y) > knee)
                y = std::copysign (knee + (1.0f - knee) * std::tanh ((std::abs (y) - knee) / (1.0f - knee)), y);
            buffer.getWritePointer (ch)[i] = y;
            peak = juce::jmax (peak, std::abs (y));
        }

        if (++delayPos >= delaySize) delayPos = 0;

        lastInHz  = voiced ? (float) sr / inPeriod : 0.0f;
        lastOutHz = voiced ? targetHz : 0.0f;
    }

    // mono-in, stereo-out: the dry copy above already mirrors channel 0.
    levelFollow = juce::jmax (peak, levelFollow * 0.85f);
    displayInHz.store (lastInHz);
    displayOutHz.store (lastOutHz);
    displayLevel.store (levelFollow);
}

void BleepProcessor::setCurrentProgram (int index)
{
    const auto& presets = bleep::factoryPresets();
    if (index < 0 || index >= (int) presets.size()) return;
    currentProgram = index;

    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());

    for (const auto& [id, value] : presets[(size_t) index].values)
        if (auto* rp = apvts.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (value));

    updateHostDisplay (ChangeDetails().withProgramChanged (true));
}

const juce::String BleepProcessor::getProgramName (int index)
{
    const auto& presets = bleep::factoryPresets();
    if (index < 0 || index >= (int) presets.size()) return {};
    return presets[(size_t) index].name;
}

juce::AudioProcessorEditor* BleepProcessor::createEditor() { return new BleepEditor (*this); }

void BleepProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, destData);
}

void BleepProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BleepProcessor(); }
