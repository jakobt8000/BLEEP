cmake_minimum_required(VERSION 3.22)
project(BLEEP VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: use a local checkout if JUCE_DIR is given, otherwise download it.
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(JUCE)
endif()

set(BLEEP_FORMATS VST3)
if(APPLE)
    list(APPEND BLEEP_FORMATS AU)
endif()

juce_add_plugin(BLEEP
    COMPANY_NAME "Sunrise Sound"
    BUNDLE_ID "dk.jakob2ft.sunrise.bleep"
    PLUGIN_MANUFACTURER_CODE Snrs
    PLUGIN_CODE Blep
    FORMATS ${BLEEP_FORMATS}
    PRODUCT_NAME "BLEEP-BLOP-VOCAL-ICER"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx Pitch Vocal)

set(BLEEP_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_sources(BLEEP PRIVATE ${BLEEP_SOURCES})
target_include_directories(BLEEP PRIVATE Source)

target_compile_definitions(BLEEP PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(BLEEP
    PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)

# Offline render test (not part of the plugin): bleep_render in.wav out.wav [param=value ...]
option(BLEEP_BUILD_TOOLS "Build offline render tool" OFF)
if(BLEEP_BUILD_TOOLS)
    juce_add_console_app(bleep_render PRODUCT_NAME "bleep_render")
    target_sources(bleep_render PRIVATE tools/render.cpp ${BLEEP_SOURCES})
    target_include_directories(bleep_render PRIVATE Source)
    target_compile_definitions(bleep_render PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JucePlugin_Name="BLEEP"
        JucePlugin_IsSynth=0 JucePlugin_WantsMidiInput=0 JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsMidiEffect=0 BLEEP_HEADLESS=1)
    target_link_libraries(bleep_render PRIVATE juce::juce_audio_utils juce::juce_dsp
        juce::juce_recommended_config_flags)
endif()
