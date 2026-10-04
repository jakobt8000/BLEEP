#pragma once
#include "PluginProcessor.h"
#include "ui/PixelFont.h"
#include <functional>

// Pixel-rasterised flat knob + paper-coloured popup menus.
class BleepLook : public juce::LookAndFeel_V4
{
public:
    BleepLook();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    juce::Font getPopupMenuFont() override;
};

// Row/grid of clickable cells bound to a choice/int parameter.
class StepSelector : public juce::Component
{
public:
    enum class Style { keys, numbers, waves, labels };
    StepSelector (juce::RangedAudioParameter& p, Style s, juce::StringArray cellText = {}, bool grid = false);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent& e) override { mouseDown (e); }
    int getIndex() const { return index; }

private:
    juce::Rectangle<float> cell (int i) const;
    int numItems() const;
    void drawWave (juce::Graphics&, int wave, juce::Rectangle<float> r, bool on);

    juce::RangedAudioParameter& param;
    Style style;
    juce::StringArray texts;
    bool grid = false;
    int index = 0;
    juce::ParameterAttachment attachment;
};

// Latching button bound to a bool parameter: FREEZE (snowflake) or a box ON/OFF switch.
class ToggleBox : public juce::Component
{
public:
    enum class Style { snowflake, onoff };
    ToggleBox (juce::RangedAudioParameter& p, Style s);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    bool isOn() const { return on; }
    std::function<void()> onChange;

private:
    juce::RangedAudioParameter& param;
    Style style;
    bool on = false;
    juce::ParameterAttachment attachment;
};

// < preset name > with a categorised popup menu.
class PresetBar : public juce::Component
{
public:
    explicit PresetBar (BleepProcessor& p) : proc (p) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void select (int index);
    juce::Rectangle<float> prevArea() const { return { 0.0f, 14.0f, 22.0f, (float) getHeight() - 15.0f }; }
    juce::Rectangle<float> nextArea() const { return { (float) getWidth() - 22.0f, 14.0f, 22.0f, (float) getHeight() - 15.0f }; }
    BleepProcessor& proc;
};

class BleepEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit BleepEditor (BleepProcessor&);
    ~BleepEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    void fillDemoTape();   // for screenshots

private:
    struct Knob
    {
        juce::Slider slider;
        juce::String label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    // one column inside a box: a knob or a custom control, with label + value text
    struct Item
    {
        juce::String label;
        std::function<juce::String()> value;
        int width = 76;
        std::function<void (juce::Rectangle<int>)> place;
        juce::Rectangle<int> col;
    };

    struct Box
    {
        juce::String name;
        ToggleBox* toggle = nullptr;     // nullptr = always on
        std::vector<Item> items;
        juce::Rectangle<int> r;
        bool isScope = false;
        int minWidth = 0;
    };

    void timerCallback() override;
    Knob& addKnob (const char* paramID, const juce::String& label, bool bipolar = false);
    Item knobItem (Knob&);
    ToggleBox* addSwitch (const char* paramID);
    template <typename T> T* own (std::unique_ptr<T> c) { auto* raw = c.get(); addAndMakeVisible (*raw); controls.push_back (std::move (c)); return raw; }
    void layoutRow (std::vector<Box*> row, int y, int h);
    void drawPaper (juce::Graphics&);
    void drawBox (juce::Graphics&, const Box&);
    void drawTape (juce::Graphics&);
    void drawReadout (juce::Graphics&);
    static juce::String noteName (float hz);

    BleepProcessor& proc;
    BleepLook look;
    std::vector<std::unique_ptr<Knob>> knobs;
    std::vector<std::unique_ptr<juce::Component>> controls;
    std::vector<Box> boxes;
    PresetBar presetBar;

    std::vector<float> histIn, histOut;   // pitch tape, in Hz (0 = silent/unvoiced)
    int histPos = 0;
    float inHz = 0.0f, outHz = 0.0f, level = 0.0f;

    juce::Rectangle<float> tapeArea, readoutArea;
    juce::Image grain, wear;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BleepEditor)
};
