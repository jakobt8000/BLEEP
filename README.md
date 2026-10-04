# BLEEP-BLOP-VOCAL-ICER

Robot voice machine from the Sunrise Sound universe. VST3 + AU for Ableton on Mac.

Crisp robotic vocals: Little AlterBoy-style pitch, formant and robot lock, plus a built-in vocoder carrier, crush and drive.

## Layout

Every effect lives in its own box with an **ON/OFF** switch in the corner. A switched-off box is printed faded, its sound is bypassed and its settings are kept. Everything starts OFF except VOICE and OUT.

| Box | Controls |
|---|---|
| VOICE | PITCH ±12 st, FORMANT ±12 st |
| ROBOT | AMOUNT (0% natural melody → 100% locked), NOTE keyboard + octave |
| SNAP | SPEED (low = natural correction, 100% = hard robot snap), KEY keyboard, scale CHR/MAJ/MIN/PEN |
| HARMONY | LEVEL, VOICES: LOW (-12), 5TH (+7), TRI (+4 +7), OCT (-12 +12) |
| VOCODER | AMOUNT, WAVE: SAW / SQUARE / BUZZ / AIR |
| RING | AMOUNT, FREQ (30 Hz = classic sci-fi robot) |
| TIN | Tuned metal-can resonance |
| RADIO | Walkie-talkie band, grit and hiss |
| WOBBLE | DEPTH, RATE: talkbox-style formant/pitch movement |
| FREEZE | Holds the voice as a drone. PITCH, ROBOT and HARMONY still play on it |
| DIRT | CRUSH (bits + sample rate), DRIVE |
| OUT | MIX, OUTPUT (soft safety limiter above -1 dBFS) |
| SCOPE | Pitch tape: grey = voice in, black = voice out |

Double-click any knob to reset it. Latency is ~33 ms and is reported to the host, so Ableton compensates automatically.

## Presets

Pick them with the arrows at the top right, or click the name for a menu. They also show up in Ableton's preset list.

- **INIT:** Init
- **CHARACTER:** Chipmunk, Giant, Tiny Helper, Basement Monster, Bright Twin, Low Twin, Whisper Ghost
- **ROBOT:** Classic Robot, Crispy Robot, Monotone Butler, Half Bot, Robot Lead, Tin Soldier
- **TUNE:** Hard Snap Major, Hard Snap Minor, Soft Correction, Pentatonic Glide, Chromatic Lock, Snap + Crunch
- **CHOIR:** Robot Choir, Octave Stack, Power Fifth, Sub Double, Snap Choir, Mothership
- **SCI-FI:** Dalek, Space Helmet, Alien Broadcast, Android, Cyborg Cop, Hologram
- **LO-FI:** Walkie-Talkie, Old Radio Robot, 8-Bit Voice, Telephone Whisper, Broken Speaker
- **FX:** Talkbox, Wah Bot, Vibrato Bot, Freeze Pad (press *), Parallel Robot

## Building

Push to GitHub: the `Build macOS` action builds a universal (Apple Silicon + Intel) VST3 and AU. Download `BLEEP-BLOP-VOCAL-ICER-mac` under the action run's **Artifacts**, then follow `INSTALL.txt`.

Local build (needs Xcode + CMake):

```
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

## Code map

- `Source/dsp/PitchDetector.h` – YIN pitch tracker on a decimated signal
- `Source/dsp/GrainShifter.h` – PSOLA pitch + formant shifter (the AlterBoy part)
- `Source/dsp/Vocoder.h` – channel vocoder with built-in oscillator
- `Source/PluginProcessor.cpp` – signal flow and parameters
- `Source/PluginEditor.cpp` – the receipt-style UI
- `tools/render.cpp` – offline test tool (`-DBLEEP_BUILD_TOOLS=ON`)
