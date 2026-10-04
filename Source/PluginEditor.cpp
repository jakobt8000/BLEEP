#include "PluginEditor.h"

using namespace bleep::ui;

namespace
{
constexpr int kW = 1000, kH = 576;
constexpr float kMargin = 44.0f;
constexpr float kTapeLoHz = 55.0f, kTapeHiHz = 880.0f;   // A1 .. A5
constexpr float kCell = 4.0f;                            // tape pixel size
constexpr int kBoxGap = 10, kBoxH = 142;

const char* kNotes[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
}

//==============================================================================
// Look and feel
BleepLook::BleepLook()
{
    setColour (juce::PopupMenu::backgroundColourId, paper);
    setColour (juce::PopupMenu::textColourId, ink);
    setColour (juce::PopupMenu::headerTextColourId, mid);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, ink);
    setColour (juce::PopupMenu::highlightedTextColourId, paper);
}

juce::Font BleepLook::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::bold));
}

void BleepLook::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                  float startAngle, float endAngle, juce::Slider& s)
{
    const float cs = 2.5f;                                  // raster cell
    const float cx = (float) x + (float) w * 0.5f, cy = (float) y + (float) h * 0.5f;
    const float r = juce::jmin ((float) w, (float) h) * 0.5f - 9.0f;
    const bool bipolar = (bool) s.getProperties().getWithDefault ("bipolar", false);

    // value dots
    const int ticks = 17;
    const float from = bipolar ? juce::jmin (0.5f, pos) : 0.0f, to = bipolar ? juce::jmax (0.5f, pos) : pos;
    for (int i = 0; i < ticks; ++i)
    {
        const float t = (float) i / (ticks - 1);
        const float a = startAngle + t * (endAngle - startAngle);
        const float px = std::round (cx + (r + 6.5f) * std::sin (a) - 1.0f);
        const float py = std::round (cy - (r + 6.5f) * std::cos (a) - 1.0f);
        const bool on = t >= from - 0.001f && t <= to + 0.001f;
        g.setColour (on ? ink : faint);
        if (on) inkPixel (g, px - 0.25f, py - 0.25f, 2.5f);
        else    g.fillRect (px, py, 2.0f, 2.0f);
    }

    // rasterised body + ring
    const int n = (int) std::ceil ((r + 2.0f) / cs);
    for (int j = -n; j <= n; ++j)
        for (int i = -n; i <= n; ++i)
        {
            const float px = (float) i * cs, py = (float) j * cs;
            const float d = std::sqrt (px * px + py * py);
            if (d < r - cs * 0.5f)
            {
                g.setColour (paper);
                g.fillRect (cx + px - cs * 0.5f, cy + py - cs * 0.5f, cs, cs);
            }
            else if (std::abs (d - r) <= cs * 0.5f)
            {
                g.setColour (ink);
                inkPixel (g, cx + px - cs * 0.5f, cy + py - cs * 0.5f, cs);
            }
        }

    // pointer, snapped to the same grid
    const float angle = startAngle + pos * (endAngle - startAngle);
    g.setColour (ink);
    juce::Point<int> last { 9999, 9999 };
    for (float t = 0.2f; t <= 0.82f; t += 0.04f)
    {
        const int gi = juce::roundToInt (t * r * std::sin (angle) / cs);
        const int gj = juce::roundToInt (-t * r * std::cos (angle) / cs);
        if (juce::Point<int> (gi, gj) == last) continue;
        last = { gi, gj };
        inkPixel (g, cx + (float) gi * cs - cs * 0.5f, cy + (float) gj * cs - cs * 0.5f, cs);
    }
}

//==============================================================================
// Step selector
StepSelector::StepSelector (juce::RangedAudioParameter& p, Style s, juce::StringArray cellText, bool useGrid)
    : param (p), style (s), texts (std::move (cellText)), grid (useGrid),
      attachment (p, [this] (float v)
                  {
                      index = juce::roundToInt (v - param.getNormalisableRange().start);
                      repaint();
                  }, nullptr)
{
    attachment.sendInitialUpdate();
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

int StepSelector::numItems() const
{
    const auto& r = param.getNormalisableRange();
    return juce::roundToInt (r.end - r.start) + 1;
}

juce::Rectangle<float> StepSelector::cell (int i) const
{
    const float w = (float) getWidth(), h = (float) getHeight();
    if (style == Style::keys)
    {
        static const int whiteCol[12] = { 0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6 };
        static const float blackCol[12] = { 0, 0.5f, 0, 1.5f, 0, 0, 3.5f, 0, 4.5f, 0, 5.5f, 0 };
        const float cw = (w - 1.5f) / 7.0f;
        if (whiteCol[i] >= 0)
            return { (float) whiteCol[i] * cw + 0.75f, 1.0f, cw, h - 2.0f };
        return { blackCol[i] * cw + 0.75f + cw * 0.22f, 1.0f, cw * 0.56f + 1.5f, h * 0.56f };
    }
    const int n = numItems();
    if (grid && n == 4)   // 2 x 2
    {
        const float gap = 3.0f, cw = (w - gap) * 0.5f, ch = (h - gap - 2.0f) * 0.5f;
        return { (float) (i % 2) * (cw + gap), 1.0f + (float) (i / 2) * (ch + gap), cw, ch };
    }
    const float gap = 3.0f, cw = (w - gap * (float) (n - 1)) / (float) n;
    return { (float) i * (cw + gap), 1.0f, cw, h - 2.0f };
}

void StepSelector::drawWave (juce::Graphics& g, int wave, juce::Rectangle<float> r, bool on)
{
    g.setColour (on ? paper : ink);
    const float px = 1.5f;
    const int cols = (int) ((r.getWidth() - 6.0f) / px), rows = (int) ((r.getHeight() - 8.0f) / px);
    const float x0 = r.getX() + (r.getWidth() - (float) cols * px) * 0.5f;
    const float y0 = r.getY() + (r.getHeight() - (float) rows * px) * 0.5f;
    int prev = -1;
    for (int c = 0; c < cols; ++c)
    {
        const float t = (float) c / (float) cols * 2.0f;   // two cycles
        const float ph = t - std::floor (t);
        float v = 0.0f;
        switch (wave)
        {
            case 0: v = 1.0f - 2.0f * ph; break;                                // saw
            case 1: v = ph < 0.5f ? 1.0f : -1.0f; break;                          // square
            case 2: v = ph < 0.15f ? 1.0f : -0.6f; break;                         // buzz
            default: v = (hash01 (c * 31, wave * 7) * 2.0f - 1.0f) * 0.9f; break; // air
        }
        const int row = juce::jlimit (0, rows - 1, juce::roundToInt ((1.0f - v) * 0.5f * (float) (rows - 1)));
        const int a = prev < 0 ? row : juce::jmin (prev, row), b = prev < 0 ? row : juce::jmax (prev, row);
        for (int rr = (wave == 3 ? row : a); rr <= (wave == 3 ? row : b); ++rr)
            g.fillRect (x0 + (float) c * px, y0 + (float) rr * px, px, px);
        prev = row;
    }
}

void StepSelector::paint (juce::Graphics& g)
{
    if (style == Style::keys)
    {
        auto key = [&] (int i, bool black)
        {
            const auto r = cell (i);
            g.setColour (black ? ink : paper);
            g.fillRect (r);
            g.setColour (ink);
            g.drawRect (r, 1.5f);
            if (i == index)   // selected: square marker at the bottom of the key
            {
                g.setColour (black ? paper : ink);
                const float m = 6.0f;
                g.fillRect (r.getCentreX() - m * 0.5f, r.getBottom() - m - 4.0f, m, m);
            }
        };
        for (int i : { 0, 2, 4, 5, 7, 9, 11 }) key (i, false);
        for (int i : { 1, 3, 6, 8, 10 })       key (i, true);
        return;
    }

    for (int i = 0; i < numItems(); ++i)
    {
        const auto r = cell (i);
        const bool on = i == index;
        g.setColour (on ? ink : paper);
        g.fillRect (r);
        g.setColour (ink);
        g.drawRect (r, 1.5f);
        g.setColour (on ? paper : ink);
        if (style == Style::numbers)
            drawText (g, juce::String (juce::roundToInt (param.getNormalisableRange().start) + i),
                      r.getCentreX(), r.getCentreY() - 5.0f, 1.5f, Align::centre);
        else if (style == Style::waves)
            drawWave (g, i, r, on);
        else
        {
            const float px = r.getHeight() < 20.0f ? 1.0f : 1.5f;
            drawText (g, texts[i], r.getCentreX(), r.getCentreY() - 3.5f * px, px, Align::centre);
        }
    }
}

void StepSelector::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.position;
    int hit = -1;
    if (style == Style::keys)
    {
        for (int i : { 1, 3, 6, 8, 10 }) if (cell (i).contains (p)) hit = i;
        if (hit < 0) for (int i : { 0, 2, 4, 5, 7, 9, 11 }) if (cell (i).contains (p)) hit = i;
    }
    else
        for (int i = 0; i < numItems(); ++i) if (cell (i).expanded (2.0f, 2.0f).contains (p)) hit = i;

    if (hit >= 0 && hit != index)
        attachment.setValueAsCompleteGesture (param.getNormalisableRange().start + (float) hit);
}

//==============================================================================
ToggleBox::ToggleBox (juce::RangedAudioParameter& p, Style s)
    : param (p), style (s),
      attachment (p, [this] (float v)
                  {
                      on = v > 0.5f; repaint();
                      if (onChange) onChange();
                  }, nullptr)
{
    attachment.sendInitialUpdate();
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void ToggleBox::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (on ? ink : paper);
    g.fillRect (r);
    g.setColour (ink);
    g.drawRect (r, 1.5f);
    g.setColour (on ? paper : ink);

    if (style == Style::onoff)
    {
        drawText (g, on ? "ON" : "OFF", r.getCentreX(), r.getCentreY() - 4.0f, 1.2f, Align::centre);
        return;
    }

    static const char* flake[] = { "#..#..#", ".#.#.#.", "..###..", "#######", "..###..", ".#.#.#.", "#..#..#" };
    const float px = 3.0f, x0 = std::round (r.getCentreX() - 3.5f * px), y0 = std::round (r.getCentreY() - 3.5f * px);
    for (int row = 0; row < 7; ++row)
        for (int c = 0; c < 7; ++c)
            if (flake[row][c] == '#') inkPixel (g, x0 + (float) c * px, y0 + (float) row * px, px);
}

void ToggleBox::mouseDown (const juce::MouseEvent&)
{
    attachment.setValueAsCompleteGesture (on ? 0.0f : 1.0f);
}

//==============================================================================
// Preset bar
void PresetBar::paint (juce::Graphics& g)
{
    const auto& presets = bleep::factoryPresets();
    const int idx = juce::jlimit (0, (int) presets.size() - 1, proc.getCurrentProgram());
    const auto& pr = presets[(size_t) idx];

    g.setColour (mid);
    drawText (g, "PRESET  " + juce::String (idx).paddedLeft ('0', 2) + "  " + pr.category, 0.0f, 0.0f, 1.2f);

    const auto row = juce::Rectangle<float> (0.0f, 14.0f, (float) getWidth(), (float) getHeight() - 15.0f);
    g.setColour (ink);
    g.drawRect (row, 1.5f);
    g.fillRect (prevArea().getRight(), row.getY(), 1.5f, row.getHeight());
    g.fillRect (nextArea().getX() - 1.5f, row.getY(), 1.5f, row.getHeight());

    auto arrow = [&] (juce::Rectangle<float> a, bool left)
    {
        const float px = 2.0f, cx = a.getCentreX(), cy = a.getCentreY();
        for (int k = 0; k < 4; ++k)
            for (int j = -k; j <= k; ++j)
                g.fillRect (std::round (cx + (left ? (float) (k - 2) : (float) (1 - k)) * px), std::round (cy + (float) j * px - 1.0f), px, px);
    };
    arrow (prevArea(), true);
    arrow (nextArea(), false);

    juce::String name = juce::String (pr.name).toUpperCase();
    const float avail = (float) getWidth() - 56.0f;
    while (textWidth (name, 1.5f) > avail && name.length() > 3) name = name.dropLastCharacters (1);
    drawText (g, name, row.getCentreX(), row.getCentreY() - 5.0f, 1.5f, Align::centre);
}

void PresetBar::select (int index)
{
    const int n = proc.getNumPrograms();
    proc.setCurrentProgram ((index % n + n) % n);
    repaint();
}

void PresetBar::mouseDown (const juce::MouseEvent& e)
{
    if (prevArea().contains (e.position)) { select (proc.getCurrentProgram() - 1); return; }
    if (nextArea().contains (e.position)) { select (proc.getCurrentProgram() + 1); return; }

    juce::PopupMenu menu;
    menu.setLookAndFeel (&getLookAndFeel());
    const auto& presets = bleep::factoryPresets();
    juce::String currentCat;
    juce::PopupMenu sub;
    auto flush = [&]
    {
        if (currentCat.isNotEmpty()) menu.addSubMenu (currentCat, sub);
        sub = {};
    };
    for (int i = 0; i < (int) presets.size(); ++i)
    {
        const juce::String cat = presets[(size_t) i].category;
        if (cat == "INIT") { menu.addItem (i + 1, "INIT", true, proc.getCurrentProgram() == i); menu.addSeparator(); continue; }
        if (cat != currentCat) { flush(); currentCat = cat; }
        sub.addItem (i + 1, juce::String (presets[(size_t) i].name).toUpperCase(), true, proc.getCurrentProgram() == i);
    }
    flush();

    juce::Component::SafePointer<PresetBar> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMinimumWidth (getWidth()),
                        [safe] (int result) { if (safe != nullptr && result > 0) safe->select (result - 1); });
}

//==============================================================================
BleepEditor::BleepEditor (BleepProcessor& p) : AudioProcessorEditor (&p), proc (p), presetBar (p)
{
    setLookAndFeel (&look);
    auto& ps = proc.apvts;
    auto param = [&] (const char* id) -> juce::RangedAudioParameter& { return *ps.getParameter (id); };

    // --- VOICE (always on)
    {
        auto& kPitch = addKnob (ParamID::pitch, "PITCH", true);
        auto& kFormant = addKnob (ParamID::formant, "FORMANT", true);
        boxes.push_back ({ "VOICE", nullptr, { knobItem (kPitch), knobItem (kFormant) } });
    }

    // --- ROBOT
    {
        auto& k = addKnob (ParamID::robot, "AMOUNT");
        auto* noteSel = own (std::make_unique<StepSelector> (param (ParamID::note), StepSelector::Style::keys));
        auto* octSel  = own (std::make_unique<StepSelector> (param (ParamID::octave), StepSelector::Style::numbers));
        Item kb { "NOTE", [noteSel, octSel] { return juce::String (kNotes[juce::jlimit (0, 11, noteSel->getIndex())]) + juce::String (octSel->getIndex() + 1); }, 124,
                  [noteSel, octSel] (juce::Rectangle<int> c) { noteSel->setBounds (c.getX() + 4, c.getY() + 48, 116, 40);
                                                               octSel->setBounds  (c.getX() + 4, c.getY() + 92, 116, 16); } };
        boxes.push_back ({ "ROBOT", addSwitch (ParamID::robotOn), { knobItem (k), kb } });
    }

    // --- SNAP
    {
        auto& k = addKnob (ParamID::snap, "SPEED");
        auto* keySel   = own (std::make_unique<StepSelector> (param (ParamID::key), StepSelector::Style::keys));
        auto* scaleSel = own (std::make_unique<StepSelector> (param (ParamID::scale), StepSelector::Style::labels,
                                                             juce::StringArray { "CHR", "MAJ", "MIN", "PEN" }));
        Item kb { "KEY", [keySel, scaleSel]
                  {
                      static const char* scales[] = { "CHROMATIC", "MAJOR", "MINOR", "PENTA" };
                      const int s = juce::jlimit (0, 3, scaleSel->getIndex());
                      return s == 0 ? juce::String ("CHROMATIC") : juce::String (kNotes[juce::jlimit (0, 11, keySel->getIndex())]) + " " + scales[s];
                  }, 124,
                  [keySel, scaleSel] (juce::Rectangle<int> c) { keySel->setBounds   (c.getX() + 4, c.getY() + 48, 116, 40);
                                                                scaleSel->setBounds (c.getX() + 4, c.getY() + 92, 116, 16); } };
        boxes.push_back ({ "SNAP", addSwitch (ParamID::snapOn), { knobItem (k), kb } });
    }

    // --- HARMONY
    {
        auto& k = addKnob (ParamID::harmony, "LEVEL");
        auto* voices = own (std::make_unique<StepSelector> (param (ParamID::voices), StepSelector::Style::labels,
                                                           juce::StringArray { "LOW", "5TH", "TRI", "OCT" }, true));
        Item v { "VOICES", [voices] { static const char* n[] = { "-12", "+7", "+4 +7", "-12 +12" }; return juce::String (n[juce::jlimit (0, 3, voices->getIndex())]); }, 116,
                 [voices] (juce::Rectangle<int> c) { voices->setBounds (c.getX() + 6, c.getY() + 56, 104, 44); } };
        boxes.push_back ({ "HARMONY", addSwitch (ParamID::harmOn), { knobItem (k), v } });
    }

    // --- VOCODER
    {
        auto& k = addKnob (ParamID::carrier, "AMOUNT");
        auto* wave = own (std::make_unique<StepSelector> (param (ParamID::wave), StepSelector::Style::waves));
        Item w { "WAVE", [wave] { static const char* n[] = { "SAW", "SQUARE", "BUZZ", "AIR" }; return juce::String (n[juce::jlimit (0, 3, wave->getIndex())]); }, 116,
                 [wave] (juce::Rectangle<int> c) { wave->setBounds (c.getX() + 4, c.getY() + 62, 108, 32); } };
        boxes.push_back ({ "VOCODER", addSwitch (ParamID::vocOn), { knobItem (k), w } });
    }

    // --- RING, TIN, RADIO, WOBBLE
    {
        auto& a = addKnob (ParamID::ring, "AMOUNT");
        auto& b = addKnob (ParamID::ringHz, "FREQ");
        boxes.push_back ({ "RING", addSwitch (ParamID::ringOn), { knobItem (a), knobItem (b) } });
    }
    boxes.push_back ({ "TIN",   addSwitch (ParamID::tinOn),   { knobItem (addKnob (ParamID::tin, "AMOUNT")) } });
    boxes.push_back ({ "RADIO", addSwitch (ParamID::radioOn), { knobItem (addKnob (ParamID::radio, "AMOUNT")) } });
    {
        auto& a = addKnob (ParamID::wobble, "DEPTH");
        auto& b = addKnob (ParamID::rate, "RATE");
        boxes.push_back ({ "WOBBLE", addSwitch (ParamID::wobOn), { knobItem (a), knobItem (b) } });
    }

    // --- FREEZE (the button is the switch)
    {
        auto* fb = own (std::make_unique<ToggleBox> (param (ParamID::freeze), ToggleBox::Style::snowflake));
        fb->onChange = [this] { repaint(); };
        Item f { "HOLD", [fb] { return juce::String (fb->isOn() ? "ON" : "OFF"); }, 76,
                 [fb] (juce::Rectangle<int> c) { fb->setBounds (c.getX() + 16, c.getY() + 56, 44, 44); } };
        boxes.push_back ({ "FREEZE", nullptr, { f } });
    }

    // --- DIRT, OUT, SCOPE
    {
        auto& a = addKnob (ParamID::crush, "CRUSH");
        auto& b = addKnob (ParamID::drive, "DRIVE");
        boxes.push_back ({ "DIRT", addSwitch (ParamID::dirtOn), { knobItem (a), knobItem (b) } });
    }
    {
        auto& a = addKnob (ParamID::mix, "MIX");
        auto& b = addKnob (ParamID::output, "OUTPUT", true);
        boxes.push_back ({ "OUT", nullptr, { knobItem (a), knobItem (b) } });
    }
    Box scope { "SCOPE", nullptr, {} };
    scope.isScope = true;
    boxes.push_back (scope);

    addAndMakeVisible (presetBar);

    setSize (kW, kH);
    timerCallback();
    startTimerHz (30);
}

BleepEditor::~BleepEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

BleepEditor::Knob& BleepEditor::addKnob (const char* paramID, const juce::String& label, bool bipolar)
{
    auto k = std::make_unique<Knob>();
    k->label = label;
    k->slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    k->slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    k->slider.setMouseDragSensitivity (180);
    k->slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    k->slider.getProperties().set ("bipolar", bipolar);
    k->slider.setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    addAndMakeVisible (k->slider);
    k->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, paramID, k->slider);
    if (auto* param = proc.apvts.getParameter (paramID))
        k->slider.setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));
    knobs.push_back (std::move (k));
    return *knobs.back();
}

BleepEditor::Item BleepEditor::knobItem (Knob& k)
{
    auto* s = &k.slider;
    return { k.label, [s] { return s->getTextFromValue (s->getValue()); }, 76,
             [s] (juce::Rectangle<int> c) { s->setBounds (c.getX() + 11, c.getY() + 50, 54, 54); } };
}

ToggleBox* BleepEditor::addSwitch (const char* paramID)
{
    auto* t = own (std::make_unique<ToggleBox> (*proc.apvts.getParameter (paramID), ToggleBox::Style::onoff));
    t->onChange = [this] { repaint(); };
    return t;
}

juce::String BleepEditor::noteName (float hz)
{
    if (hz <= 0.0f) return "--";
    const int midi = juce::roundToInt (69.0f + 12.0f * std::log2 (hz / 440.0f));
    return juce::String (kNotes[(midi % 12 + 12) % 12]) + juce::String (midi / 12 - 1);
}

void BleepEditor::timerCallback()
{
    const float i = proc.displayInHz.load(), o = proc.displayOutHz.load();
    inHz  = i > 0.0f ? (inHz  > 0.0f ? inHz  * 0.6f + i * 0.4f : i) : 0.0f;
    outHz = o > 0.0f ? (outHz > 0.0f ? outHz * 0.6f + o * 0.4f : o) : 0.0f;
    level = proc.displayLevel.load();

    if (! histIn.empty())
    {
        histIn[(size_t) histPos] = inHz;
        histOut[(size_t) histPos] = outHz;
        histPos = (histPos + 1) % (int) histIn.size();
    }
    repaint();
}

void BleepEditor::fillDemoTape()
{
    for (size_t k = 0; k < histIn.size(); ++k)
    {
        const float t = (float) k / (float) histIn.size();
        const bool gap = (k % 40) > 33;
        histIn[k]  = gap ? 0.0f : 190.0f * std::pow (2.0f, 0.35f * std::sin (t * 8.0f) + 0.15f * std::sin (t * 27.0f));
        histOut[k] = gap ? 0.0f : (t < 0.45f ? histIn[k] * 0.75f : 130.8f);
    }
    histPos = 0;
    inHz = 196.0f; outHz = 130.8f; level = 0.35f;
}

//==============================================================================
void BleepEditor::drawPaper (juce::Graphics& g)
{
    g.fillAll (desk);

    juce::Path p;
    const float x0 = 16.0f, x1 = (float) kW - 16.0f, y0 = 10.0f, y1 = (float) kH - 10.0f, tooth = 10.0f, depth = 5.0f;
    p.startNewSubPath (x0, y0 + depth);
    for (float x = x0; x < x1; x += tooth)
    {
        p.lineTo (x + tooth * 0.5f, y0);
        p.lineTo (juce::jmin (x + tooth, x1), y0 + depth);
    }
    p.lineTo (x1, y1 - depth);
    for (float x = x1; x > x0; x -= tooth)
    {
        p.lineTo (x - tooth * 0.5f, y1);
        p.lineTo (juce::jmax (x - tooth, x0), y1 - depth);
    }
    p.closeSubPath();

    g.setColour (juce::Colours::black.withAlpha (0.10f));
    g.fillPath (p, juce::AffineTransform::translation (0.0f, 3.0f));
    g.setColour (paper);
    g.fillPath (p);

    if (grain.isValid()) g.drawImageAt (grain, 0, 0);
}

void BleepEditor::drawBox (juce::Graphics& g, const Box& b)
{
    const auto r = b.r.toFloat();
    g.setColour (ink);
    g.drawRect (r, 1.5f);
    drawText (g, b.name, r.getX() + 10.0f, r.getY() + 9.0f, 1.5f);

    g.setColour (faint);
    for (float x = r.getX() + 6.0f; x < r.getRight() - 6.0f; x += 4.0f) g.fillRect (x, r.getY() + 28.0f, 2.0f, 1.0f);

    for (const auto& it : b.items)
    {
        const auto c = it.col.toFloat();
        g.setColour (ink);
        drawText (g, it.label, c.getCentreX(), c.getY() + 36.0f, 1.5f, Align::centre);
        g.setColour (mid);
        if (it.value) drawText (g, it.value(), c.getCentreX(), c.getY() + 118.0f, 1.5f, Align::centre);
    }
}

void BleepEditor::drawTape (juce::Graphics& g)
{
    const auto a = tapeArea;
    auto hzToY = [&] (float hz)
    {
        const float t = std::log2 (hz / kTapeLoHz) / std::log2 (kTapeHiHz / kTapeLoHz);
        const int rows = (int) (a.getHeight() / kCell);
        const int row = juce::jlimit (0, rows - 1, (int) std::round ((1.0f - t) * (float) (rows - 1)));
        return a.getY() + (float) row * kCell;
    };

    for (int oct = 2; oct <= 5; ++oct)
    {
        const float hz = 440.0f * std::pow (2.0f, (float) (12 * (oct + 1) - 69) / 12.0f);
        const float y = hzToY (hz) + kCell * 0.5f;
        g.setColour (faint);
        for (float x = a.getX() + 24.0f; x < a.getRight(); x += 8.0f) g.fillRect (x, y, 2.0f, 1.0f);
        g.setColour (mid);
        drawText (g, "C" + juce::String (oct), a.getX(), y - 4.0f, 1.2f);
    }

    const int n = (int) histIn.size();
    const float x0 = a.getX() + 24.0f;
    float lastY = -1.0f;
    for (int k = 0; k < n; ++k)
    {
        const int idx = (histPos + k) % n;
        const float x = x0 + (float) k * kCell;
        if (x + kCell > a.getRight()) break;

        if (histIn[(size_t) idx] > 0.0f)
        {
            g.setColour (mid);
            g.fillRect (x + 1.0f, hzToY (histIn[(size_t) idx]) + 1.0f, kCell - 2.0f, kCell - 2.0f);
        }
        if (histOut[(size_t) idx] > 0.0f)
        {
            const float y = hzToY (histOut[(size_t) idx]);
            g.setColour (ink);
            inkPixel (g, x, y, kCell);
            if (lastY >= 0.0f && std::abs (lastY - y) > kCell)
                for (float yy = juce::jmin (lastY, y) + kCell; yy < juce::jmax (lastY, y); yy += kCell)
                    inkPixel (g, x, yy, kCell);
            lastY = y;
        }
        else lastY = -1.0f;
    }
}

void BleepEditor::drawReadout (juce::Graphics& g)
{
    const auto r = readoutArea;
    g.setColour (ink);
    drawText (g, "OUT", r.getX(), r.getY(), 1.5f);
    drawText (g, noteName (outHz), r.getX(), r.getY() + 18.0f, 3.0f);
    drawText (g, outHz > 0.0f ? juce::String (outHz, 1) + " HZ" : juce::String ("---- HZ"), r.getX(), r.getY() + 46.0f, 1.5f);
    g.setColour (mid);
    drawText (g, "IN  " + noteName (inHz), r.getX(), r.getY() + 66.0f, 1.5f);

    // legend
    g.fillRect (r.getX(), r.getY() + 88.0f, 3.0f, 3.0f);
    drawText (g, "IN", r.getX() + 7.0f, r.getY() + 86.0f, 1.2f);
    g.setColour (ink);
    g.fillRect (r.getX() + 34.0f, r.getY() + 87.0f, 4.0f, 4.0f);
    drawText (g, "OUT", r.getX() + 42.0f, r.getY() + 86.0f, 1.2f);
}

void BleepEditor::paint (juce::Graphics& g)
{
    drawPaper (g);
    g.setColour (ink);
    drawLogo (g, "BLEEP-BLOP-VOCAL-ICER", kMargin, 26.0f, 3.5f);

    for (const auto& b : boxes)
    {
        drawBox (g, b);
        if (b.isScope) { drawTape (g); drawReadout (g); }
    }

    g.setColour (mid);
    drawText (g, "DBL-CLICK = RESET   *** THANK YOU ***", (float) kW - kMargin, (float) kH - 30.0f, 1.5f, Align::right);
}

void BleepEditor::paintOverChildren (juce::Graphics& g)
{
    // switched-off boxes are printed "faded"
    for (const auto& b : boxes)
        if (b.toggle != nullptr && ! b.toggle->isOn())
        {
            const auto r = b.r.toFloat();
            g.setColour (paper.withAlpha (0.68f));
            g.fillRect (r.getX() + 1.5f, r.getY() + 30.0f, r.getWidth() - 3.0f, r.getHeight() - 31.5f);
            g.fillRect (r.getX() + 1.5f, r.getY() + 1.5f, r.getRight() - 46.0f - r.getX(), 27.0f);
        }

    if (wear.isValid()) g.drawImageAt (wear, 0, 0);   // worn print: paper specks over the ink
}

void BleepEditor::layoutRow (std::vector<Box*> row, int y, int h)
{
    const int inner = kW - 2 * (int) kMargin;
    int contentTotal = 0, scopeCount = 0;
    for (auto* b : row)
    {
        int w = 16;
        for (auto& it : b->items) w += it.width;
        b->minWidth = juce::jmax (w, b->minWidth);
        contentTotal += b->minWidth;
        if (b->isScope) ++scopeCount;
    }
    const int gaps = kBoxGap * ((int) row.size() - 1);
    const int extra = juce::jmax (0, inner - gaps - contentTotal);

    int x = (int) kMargin;
    for (size_t i = 0; i < row.size(); ++i)
    {
        auto* b = row[i];
        int w = b->minWidth;
        if (scopeCount > 0) { if (b->isScope) w += extra / scopeCount; }
        else w += extra / (int) row.size();
        if (i + 1 == row.size()) w = (int) kMargin + inner - x;   // absorb rounding
        b->r = { x, y, w, h };

        int content = 0;
        for (auto& it : b->items) content += it.width;
        int cx = x + (w - content) / 2;
        for (auto& it : b->items)
        {
            it.col = { cx, y, it.width, h };
            if (it.place) it.place (it.col);
            cx += it.width;
        }
        if (b->toggle != nullptr) b->toggle->setBounds (b->r.getRight() - 42, y + 6, 34, 17);
        x += w + kBoxGap;
    }
}

void BleepEditor::resized()
{
    presetBar.setBounds (kW - (int) kMargin - 230, 22, 230, 44);

    auto find = [&] (const char* n) -> Box* { for (auto& b : boxes) if (b.name == n) return &b; return nullptr; };
    const int y0 = 84;
    find ("TIN")->minWidth = 108;
    find ("RADIO")->minWidth = 108;
    layoutRow ({ find ("VOICE"), find ("ROBOT"), find ("SNAP"), find ("HARMONY") }, y0, kBoxH);
    layoutRow ({ find ("VOCODER"), find ("RING"), find ("TIN"), find ("RADIO"), find ("WOBBLE"), find ("FREEZE") }, y0 + kBoxH + kBoxGap, kBoxH);
    auto* scope = find ("SCOPE");
    scope->minWidth = 200;
    layoutRow ({ find ("DIRT"), find ("OUT"), scope }, y0 + 2 * (kBoxH + kBoxGap), kBoxH);

    const auto sr = scope->r.toFloat();
    readoutArea = { sr.getRight() - 150.0f, sr.getY() + 38.0f, 140.0f, 96.0f };
    tapeArea    = { sr.getX() + 12.0f, sr.getY() + 38.0f, readoutArea.getX() - sr.getX() - 28.0f, 96.0f };

    const int cols = juce::jmax (8, (int) ((tapeArea.getWidth() - 24.0f) / kCell));
    histIn.assign ((size_t) cols, 0.0f);
    histOut.assign ((size_t) cols, 0.0f);
    histPos = 0;

    // print texture (fixed seed so it never "swims")
    juce::Random rnd (2026);
    grain = juce::Image (juce::Image::ARGB, kW, kH, true);
    {
        juce::Graphics gg (grain);
        for (int i = 0; i < 3200; ++i)
        {
            gg.setColour (ink.withAlpha (0.03f + 0.07f * rnd.nextFloat()));
            gg.fillRect (16.0f + rnd.nextFloat() * (kW - 32), 14.0f + rnd.nextFloat() * (kH - 28), 1.0f, 1.0f);
        }
        for (int i = 0; i < 90; ++i)
        {
            const float fx = 20.0f + rnd.nextFloat() * (kW - 40), fy = 16.0f + rnd.nextFloat() * (kH - 32);
            const float a = rnd.nextFloat() * juce::MathConstants<float>::twoPi, len = 6.0f + rnd.nextFloat() * 14.0f;
            gg.setColour (ink.withAlpha (0.035f));
            gg.drawLine (fx, fy, fx + len * std::cos (a), fy + len * std::sin (a), 0.7f);
        }
    }
    wear = juce::Image (juce::Image::ARGB, kW, kH, true);
    {
        juce::Graphics gg (wear);
        for (int i = 0; i < 1800; ++i)
        {
            gg.setColour (paper.withAlpha (0.45f + 0.4f * rnd.nextFloat()));
            const float s = 0.8f + rnd.nextFloat() * 0.9f;
            gg.fillRect (20.0f + rnd.nextFloat() * (kW - 40), 16.0f + rnd.nextFloat() * (kH - 32), s, s);
        }
    }
}
