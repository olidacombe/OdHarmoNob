/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "JuceHeader.h"
#include "Pfft.h"


class freqMultiplySpectrumCBO : public OdPfft::Pfft<float>::frequencyDomainCallbackObject
{
public:
    freqMultiplySpectrumCBO(int s, int c) : size(s), numChannels(c), factor(1.0) {}
    ~freqMultiplySpectrumCBO() override
    {
        //roBuf.clear();
    }
    void setFactor(float f) { factor = f; };
    void spectrumCallback(AudioBuffer<float>& buf) override;
private:
    int size;
    int numChannels;
    float factor;
    AudioBuffer<float> roBuf;
    
};


//==============================================================================
/**
*/
class OdHarmoNobAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    OdHarmoNobAudioProcessor();
    ~OdHarmoNobAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet) override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //void spectrumCallback(AudioBuffer<float>& spectrum);
/*
    enum
    {
        numFFTs = 4,
        fftOrder = 10,
        fftSize = 1 << fftOrder,
        //fftOverlap = fftSize >> 1
    };
    
    const int hopSize = fftSize / numFFTs;
*/
    //ScopedPointer<FFT> FFTs[numFFTs];
    ScopedPointer<OdPfft::Pfft<float>> pfft;
    ScopedPointer<freqMultiplySpectrumCBO> spectrumCallbackObject;
    
    //OwnedArray<Pfft> pffts;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OdHarmoNobAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
