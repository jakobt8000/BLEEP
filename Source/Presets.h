#pragma once
#include <vector>
#include <utility>

// Factory presets. Values are in real parameter units; anything not listed
// is reset to its default (and every effect box defaults to OFF).
namespace bleep
{
struct Preset
{
    const char* category;
    const char* name;
    std::vector<std::pair<const char*, float>> values;
};

inline const std::vector<Preset>& factoryPresets()
{
    // notes: 0=C 1=C# 2=D 3=D# 4=E 5=F 6=F# 7=G 8=G# 9=A 10=A# 11=B
    // scale: 0=CHR 1=MAJ 2=MIN 3=PEN   voices: 0=LOW 1=5TH 2=TRI 3=OCT   wave: 0=SAW 1=SQR 2=BUZZ 3=AIR
    static const std::vector<Preset> p {
        { "INIT", "Init", {} },

        { "CHARACTER", "Chipmunk",          { { "pitch", 7 }, { "formant", 6 } } },
        { "CHARACTER", "Giant",             { { "pitch", -7 }, { "formant", -6 } } },
        { "CHARACTER", "Tiny Helper",       { { "pitch", 12 }, { "formant", 9 } } },
        { "CHARACTER", "Basement Monster",  { { "pitch", -12 }, { "formant", -8 }, { "dirtOn", 1 }, { "drive", 30 } } },
        { "CHARACTER", "Bright Twin",       { { "pitch", 4 }, { "formant", 4 } } },
        { "CHARACTER", "Low Twin",          { { "pitch", -4 }, { "formant", -3 } } },
        { "CHARACTER", "Whisper Ghost",     { { "vocOn", 1 }, { "carrier", 100 }, { "wave", 3 }, { "formant", 3 },
                                              { "tinOn", 1 }, { "tin", 30 }, { "wobOn", 1 }, { "wobble", 20 }, { "rate", 0.3f }, { "output", -3 } } },

        { "ROBOT", "Classic Robot",         { { "robotOn", 1 }, { "robot", 100 }, { "note", 0 }, { "octave", 3 } } },
        { "ROBOT", "Crispy Robot",          { { "robotOn", 1 }, { "robot", 100 }, { "vocOn", 1 }, { "carrier", 50 }, { "wave", 2 },
                                              { "dirtOn", 1 }, { "crush", 25 }, { "drive", 40 } } },
        { "ROBOT", "Monotone Butler",       { { "robotOn", 1 }, { "robot", 100 }, { "note", 7 }, { "octave", 2 }, { "formant", -2 },
                                              { "tinOn", 1 }, { "tin", 30 } } },
        { "ROBOT", "Half Bot",              { { "robotOn", 1 }, { "robot", 60 } } },
        { "ROBOT", "Robot Lead",            { { "robotOn", 1 }, { "robot", 100 }, { "octave", 4 }, { "vocOn", 1 }, { "carrier", 60 },
                                              { "dirtOn", 1 }, { "drive", 20 } } },
        { "ROBOT", "Tin Soldier",           { { "robotOn", 1 }, { "robot", 100 }, { "note", 2 }, { "tinOn", 1 }, { "tin", 70 },
                                              { "radioOn", 1 }, { "radio", 25 } } },

        { "TUNE", "Hard Snap Major",        { { "snapOn", 1 }, { "snap", 100 }, { "key", 0 }, { "scale", 1 } } },
        { "TUNE", "Hard Snap Minor",        { { "snapOn", 1 }, { "snap", 100 }, { "key", 9 }, { "scale", 2 } } },
        { "TUNE", "Soft Correction",        { { "snapOn", 1 }, { "snap", 45 }, { "key", 0 }, { "scale", 1 } } },
        { "TUNE", "Pentatonic Glide",       { { "snapOn", 1 }, { "snap", 75 }, { "key", 0 }, { "scale", 3 } } },
        { "TUNE", "Chromatic Lock",         { { "snapOn", 1 }, { "snap", 100 }, { "scale", 0 } } },
        { "TUNE", "Snap + Crunch",          { { "snapOn", 1 }, { "snap", 100 }, { "scale", 2 }, { "key", 9 },
                                              { "dirtOn", 1 }, { "drive", 45 }, { "crush", 15 } } },

        { "CHOIR", "Robot Choir",           { { "robotOn", 1 }, { "robot", 100 }, { "harmOn", 1 }, { "harmony", 80 }, { "voices", 2 },
                                              { "vocOn", 1 }, { "carrier", 40 } } },
        { "CHOIR", "Octave Stack",          { { "harmOn", 1 }, { "harmony", 70 }, { "voices", 3 }, { "output", -2 } } },
        { "CHOIR", "Power Fifth",           { { "harmOn", 1 }, { "harmony", 70 }, { "voices", 1 }, { "dirtOn", 1 }, { "drive", 35 } } },
        { "CHOIR", "Sub Double",            { { "harmOn", 1 }, { "harmony", 60 }, { "voices", 0 } } },
        { "CHOIR", "Snap Choir",            { { "snapOn", 1 }, { "snap", 100 }, { "scale", 1 }, { "harmOn", 1 }, { "harmony", 70 }, { "voices", 2 } } },
        { "CHOIR", "Mothership",            { { "pitch", -5 }, { "formant", -4 }, { "harmOn", 1 }, { "harmony", 60 }, { "voices", 3 },
                                              { "vocOn", 1 }, { "carrier", 50 }, { "wobOn", 1 }, { "wobble", 20 }, { "rate", 0.5f } } },

        { "SCI-FI", "Dalek",                { { "pitch", -3 }, { "ringOn", 1 }, { "ring", 80 }, { "ringhz", 30 }, { "dirtOn", 1 }, { "drive", 20 } } },
        { "SCI-FI", "Space Helmet",         { { "snapOn", 1 }, { "snap", 80 }, { "tinOn", 1 }, { "tin", 50 }, { "ringOn", 1 }, { "ring", 25 }, { "ringhz", 90 } } },
        { "SCI-FI", "Alien Broadcast",      { { "ringOn", 1 }, { "ring", 60 }, { "ringhz", 400 }, { "radioOn", 1 }, { "radio", 60 },
                                              { "wobOn", 1 }, { "wobble", 40 }, { "rate", 5 } } },
        { "SCI-FI", "Android",              { { "robotOn", 1 }, { "robot", 80 }, { "vocOn", 1 }, { "carrier", 70 }, { "wave", 1 }, { "formant", 2 } } },
        { "SCI-FI", "Cyborg Cop",           { { "pitch", -2 }, { "formant", -2 }, { "robotOn", 1 }, { "robot", 40 }, { "tinOn", 1 }, { "tin", 45 },
                                              { "ringOn", 1 }, { "ring", 20 }, { "ringhz", 55 } } },
        { "SCI-FI", "Hologram",             { { "harmOn", 1 }, { "harmony", 45 }, { "voices", 3 }, { "ringOn", 1 }, { "ring", 30 }, { "ringhz", 800 },
                                              { "wobOn", 1 }, { "wobble", 25 }, { "rate", 7 } } },

        { "LO-FI", "Walkie-Talkie",         { { "radioOn", 1 }, { "radio", 90 } } },
        { "LO-FI", "Old Radio Robot",       { { "robotOn", 1 }, { "robot", 100 }, { "radioOn", 1 }, { "radio", 70 },
                                              { "dirtOn", 1 }, { "crush", 30 }, { "drive", 30 } } },
        { "LO-FI", "8-Bit Voice",           { { "robotOn", 1 }, { "robot", 100 }, { "octave", 4 }, { "vocOn", 1 }, { "carrier", 100 }, { "wave", 1 },
                                              { "dirtOn", 1 }, { "crush", 60 }, { "output", -4 } } },
        { "LO-FI", "Telephone Whisper",     { { "vocOn", 1 }, { "carrier", 100 }, { "wave", 3 }, { "radioOn", 1 }, { "radio", 60 } } },
        { "LO-FI", "Broken Speaker",        { { "dirtOn", 1 }, { "drive", 80 }, { "crush", 40 }, { "radioOn", 1 }, { "radio", 40 } } },

        { "FX", "Talkbox",                  { { "vocOn", 1 }, { "carrier", 80 }, { "wave", 0 }, { "wobOn", 1 }, { "wobble", 50 }, { "rate", 2.5f },
                                              { "snapOn", 1 }, { "snap", 60 } } },
        { "FX", "Wah Bot",                  { { "robotOn", 1 }, { "robot", 70 }, { "wobOn", 1 }, { "wobble", 80 }, { "rate", 1.5f } } },
        { "FX", "Vibrato Bot",              { { "robotOn", 1 }, { "robot", 100 }, { "wobOn", 1 }, { "wobble", 35 }, { "rate", 6 } } },
        { "FX", "Freeze Pad (press *)",     { { "harmOn", 1 }, { "harmony", 70 }, { "voices", 2 }, { "vocOn", 1 }, { "carrier", 30 },
                                              { "wobOn", 1 }, { "wobble", 15 }, { "rate", 0.4f } } },
        { "FX", "Parallel Robot",           { { "robotOn", 1 }, { "robot", 100 }, { "vocOn", 1 }, { "carrier", 60 }, { "mix", 50 } } },
    };
    return p;
}
} // namespace bleep
