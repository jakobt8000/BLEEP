// Offline test renderer: bleep_render in.wav out.wav [param=value ...]
// Values are in the parameter's real units (e.g. pitch=-5 robot=100 wave=2).
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_formats/juce_audio_formats.h>

int main (int argc, char** argv)
{
    if (argc < 3) { std::puts ("usage: bleep_render in.wav out.wav [param=value ...]"); return 1; }
    juce::ScopedJuceInitialiser_GUI init;

    if (juce::String (argv[1]) == "--snapshot")   // bleep_render --snapshot out.png [param=value ...]
    {
        BleepProcessor proc;
        for (int a = 3; a < argc; ++a)
        {
            auto kv = juce::StringArray::fromTokens (argv[a], "=", "");
            if (kv[0] == "program") { proc.setCurrentProgram (kv[1].getIntValue()); continue; }
            if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (proc.apvts.getParameter (kv[0])))
                p->setValueNotifyingHost (p->convertTo0to1 (kv[1].getFloatValue()));
        }
        proc.displayInHz = 196.0f; proc.displayOutHz = 130.8f; proc.displayLevel = 0.3f;
        std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
        if (auto* be = dynamic_cast<BleepEditor*> (ed.get())) be->fillDemoTape();
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f);
        juce::File f { juce::String (argv[2]) }; f.deleteFile();
        juce::FileOutputStream os (f); juce::PNGImageFormat().writeImageToStream (img, os);
        return 0;
    }

    juce::AudioFormatManager fm; fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (juce::File (juce::String (argv[1]))));
    if (! reader) { std::puts ("cannot read input"); return 1; }

    const int len = (int) reader->lengthInSamples;
    juce::AudioBuffer<float> audio (2, len);
    reader->read (&audio, 0, len, 0, true, true);

    BleepProcessor proc;
    double freezeAt = -1.0;
    for (int a = 3; a < argc; ++a)
    {
        auto kv = juce::StringArray::fromTokens (argv[a], "=", "");
        if (kv[0] == "freeze_at") { freezeAt = kv[1].getDoubleValue(); continue; }
        if (kv[0] == "program") { proc.setCurrentProgram (kv[1].getIntValue()); continue; }
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (proc.apvts.getParameter (kv[0])))
            p->setValueNotifyingHost (p->convertTo0to1 (kv[1].getFloatValue()));
        else
            std::printf ("unknown param %s\n", kv[0].toRawUTF8());
    }

    proc.setPlayConfigDetails (2, 2, reader->sampleRate, 512);
    proc.prepareToPlay (reader->sampleRate, 512);
    const int lat = proc.getLatencySamples();

    juce::AudioBuffer<float> out (2, len + lat);
    out.clear();
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> block (2, 512);
    for (int pos = 0; pos < len + lat; pos += 512)
    {
        const int n = juce::jmin (512, len + lat - pos);
        block.setSize (2, n, false, false, true);
        block.clear();
        for (int ch = 0; ch < 2; ++ch)
            if (pos < len) block.copyFrom (ch, 0, audio, ch, pos, juce::jmin (n, len - pos));
        if (freezeAt >= 0.0 && pos >= (int) (freezeAt * reader->sampleRate))
        {
            proc.apvts.getParameter ("freeze")->setValueNotifyingHost (1.0f);
            freezeAt = -1.0;
        }
        proc.processBlock (block, midi);
        for (int ch = 0; ch < 2; ++ch) out.copyFrom (ch, pos, block, ch, 0, n);
    }

    juce::File outFile { juce::String (argv[2]) };
    outFile.deleteFile();
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> writer (wav.createWriterFor (new juce::FileOutputStream (outFile),
                                                     reader->sampleRate, 2, 24, {}, 0));
    writer->writeFromAudioSampleBuffer (out, lat, len);   // latency-compensated
    std::printf ("latency %d samples, wrote %s\n", lat, argv[2]);
    return 0;
}
