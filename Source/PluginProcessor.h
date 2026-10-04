#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "dsp/PitchDetector.h"
#include "dsp/GrainShifter.h"
#include "dsp/Vocoder.h"
#include "dsp/ColorFx.h"
#include "Presets.h"
#include <atomic>

namespace ParamID
{
    static constexpr const char* pitch   = "pitch";
    static constexpr const char* formant = "formant";
    static constexpr const char* robot   = "robot";
    static constexpr const char* note    = "note";
    static constexpr const char* octave  = "octave";
    static constexpr const char* carrier = "carrier";
    static constexpr const char* wave    = "wave";
    static constexpr const char* crush   = "crush";
    static constexpr const char* drive   = "drive";
    static constexpr const char* mix     = "mix";
    static constexpr const char* output  = "output";
    static constexpr const char* snap    = "snap";
    static constexpr const char* scale   = "scale";
    static constexpr const char* harmony = "harmony";
    static constexpr const char* voices  = "voices";
    static constexpr const char* ring    = "ring";
    static constexpr const char* ringHz  = "ringhz";
    static constexpr const char* tin     = "tin";
    static constexpr const char* radio   = "radio";
    static constexpr const char* wobble  = "wobble";
    static constexpr const char* rate    = "rate";
    static constexpr const char* freeze  = "freeze";
    static constexpr const char* key     = "key";
    static constexpr const char* robotOn = "robotOn";
    static constexpr const char* snapOn  = "snapOn";
    static constexpr const char* harmOn  = "harmOn";
    static constexpr const char* vocOn   = "vocOn";
    static constexpr const char* ringOn  = "ringOn";
    static constexpr const char* tinOn   = "tinOn";
    static constexpr const char* radioOn = "radioOn";
    static constexpr const char* wobOn   = "wobOn";
    static constexpr const char* dirtOn  = "dirtOn";
}

class BleepProcessor : public juce::AudioProcessor
{
public:
    BleepProcessor();
    ~BleepProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    using AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "BLEEP-BLOP-VOCAL-ICER"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.05; }

    int getNumPrograms() override { return (int) bleep::factoryPresets().size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts;

    // for the display
    std::atomic<float> displayInHz  { 0.0f };
    std::atomic<float> displayOutHz { 0.0f };
    std::atomic<float> displayLevel { 0.0f };

private:
    bleep::PitchDetector detector;
    bleep::GrainShifter shifter;
    bleep::Vocoder vocoder;
    bleep::GrainShifter harm1, harm2;
    bleep::RingMod ringMod;
    bleep::TinCan tinCan;
    bleep::Radio radioFx;

    float snappedMidi = 50.0f, snapCoefBase = 0.0f;
    double wobblePhase = 0.0;
    bool prevFreeze = false, frozenVoiced = false; float frozenPeriod = 300.0f, frozenLogIn = 7.2f;

    // delay lines that keep everything aligned with the shifter's latency
    juce::AudioBuffer<float> dryDelay;
    std::vector<float> monoDelay, hzDelay, uvDelay;
    int delayPos = 0, delaySize = 1, latency = 0;

    float lastLogIn = 7.2f;   // log2(Hz) of last voiced input (~147 Hz)
    float carrierHzSmoothed = 147.0f, unvoicedSmoothed = 1.0f;
    float glideCoef = 0.99f, uvCoef = 0.99f;

    float crushHold = 0.0f;
    int crushCounter = 0;
    float levelFollow = 0.0f;

    juce::SmoothedValue<float> mixS, outS, carrierS, driveS, harmS, ringS, tinS, radioS, robotS, wobS;

    std::atomic<float>* pPitch = nullptr; std::atomic<float>* pFormant = nullptr;
    std::atomic<float>* pRobot = nullptr; std::atomic<float>* pNote = nullptr;
    std::atomic<float>* pOctave = nullptr; std::atomic<float>* pCarrier = nullptr;
    std::atomic<float>* pWave = nullptr; std::atomic<float>* pCrush = nullptr;
    std::atomic<float>* pDrive = nullptr; std::atomic<float>* pMix = nullptr;
    std::atomic<float>* pOutput = nullptr;
    std::atomic<float>* pSnap = nullptr; std::atomic<float>* pScale = nullptr;
    std::atomic<float>* pHarmony = nullptr; std::atomic<float>* pVoices = nullptr;
    std::atomic<float>* pRing = nullptr; std::atomic<float>* pRingHz = nullptr;
    std::atomic<float>* pTin = nullptr; std::atomic<float>* pRadio = nullptr;
    std::atomic<float>* pWobble = nullptr; std::atomic<float>* pRate = nullptr;
    std::atomic<float>* pFreeze = nullptr;
    std::atomic<float>* pKey = nullptr;
    std::atomic<float>* pRobotOn = nullptr; std::atomic<float>* pSnapOn = nullptr; std::atomic<float>* pHarmOn = nullptr;
    std::atomic<float>* pVocOn = nullptr; std::atomic<float>* pRingOn = nullptr; std::atomic<float>* pTinOn = nullptr;
    std::atomic<float>* pRadioOn = nullptr; std::atomic<float>* pWobOn = nullptr; std::atomic<float>* pDirtOn = nullptr;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BleepProcessor)
};
