/*
 ==============================================================================
 
 PluginProcessor.h
 Created from a JUCE template.
 Author:  Olly Seber
 
 ==============================================================================
 */

/*
 Class for audio processor. This class manages all the DSP.
 
 DELAY AND FILTER
 Uses two custom member objects:
 LPFilter and VariableDelayLine. A HeapBlock<float> is used to store samples for feedback. This
 has been implemented in this way so the plugin can support multiple channels, and is fairly
 extendable.
 
 LFOs
 In addition, uses JUCE dsp::Oscillator objects as LFOs for modulating the delay time.
 
 THIS REQUIRES C++ 14. THIS IS ENABLED IN THE PROJUCER PROJECT, BUT IF ENCOUNTERING BUILD ERRORS
 PLEASE DOUBLE CHECK THE SETTINGS.
 
 PARAMETERS
 All parameters are managed by the ValueTreeState object, and use the LinearSmoothedValue class
 to keep transitions smoooooth.
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DelayDSP.h"
#include "FilterDSP.h"

//==============================================================================

class CwdelayAudioProcessor  : public AudioProcessor,
                               public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    /* CONSTRUCTOR AND DESTRUCTOR */
    //==============================================================================
    CwdelayAudioProcessor();
    ~CwdelayAudioProcessor();
    
    //==============================================================================
    /* AUDIO FUNCTIONS */
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    /* GUI/'EDITOR' FUNCTIONS */
    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    /* PLUGIN INFO FUNCTIONS */
    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect () const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    /* PROGRAM FUNCTIONS (programs are not currently supported) */
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    /* STATE STORAGE/RECAL FUNCTIONS */
    /* Store and recalls the current state of the plugin so the setting remain
     * when the GUI is closed/reopened. */
    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    /* CALLBACK FUNCTION FROM GUI */
    /* Called when a parameter is changed on the GUI. */
    //==============================================================================
    void parameterChanged(const String& parameterID, float newValue) override;
    
private:
    //==============================================================================
    /* MEMBERS FOR PLUGIN PARAMETERS */
    //==============================================================================
    /* Static functions for parameter text to float and float to text. */
    static String onOffFloatToText (float value);
    static float onOffTextToFloat (const String& text);
    
    /* ValueTreeState object manages plugin parameters. */
    AudioProcessorValueTreeState parameters;
    
    /* Use linear smoothing of parameter changes. */
    LinearSmoothedValue<float> delaySize;
    LinearSmoothedValue<float> wetLevel;
    LinearSmoothedValue<float> feedback;
    
    float samplerate; // Required for real-time calculation of delay size in samples.
    
    /* For smooooothing gain changes. */
    float previousInputGain;
    float previousOutputGain;
    
    //==============================================================================
    /* MEMBERS FOR DSP */
    //==============================================================================
    VariableDelayLine delay;
    LPFilter filter;
    
    HeapBlock<float> out; // For storing feedback samples
    
    //==============================================================================
    /* MEMBERS FOR LFOS */
    //==============================================================================
    /* LFO functions for varying delay line.
     * Must be static as function pointer is used. */
    static float LFOFunc(float angle);
    static float LFO1Func(float angle);
    
    /* LFO parameters.
     * Must also be static in order to be used by above functions.
     * constexpr as can be evaluated at compile time. */
    static constexpr float LFOdepth = 0.001; // in mSec
    static constexpr float LFO1depth = 0.01; // in mSec
    
    /* LFOs will vary the delay size, so be sure to take this into account when setting delay buffer length.
     * LFOs vary by a magnitude of LFOdepth around a centre point LFOdepth - they are unipolar.
     * Therefore max value is actually 2x LFOdepth for each LFO, hence multiply by two.
     */
    static constexpr float maxLFOOffset = 2.f * (LFOdepth + LFO1depth);
    
    /* Used at object instantiation, so not required to be static, but must be above the LFO members. */
    const int LFOResolution = 50;
    
    /* Must be instantiated after LFOResolution is initialised. */
    dsp::Oscillator<float> LFO;
    dsp::Oscillator<float> LFO1;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CwdelayAudioProcessor)
};
