/*
  ==============================================================================

    Pfft.h
    Created: 29 Jun 2016 9:24:41pm
    Author:  Oli

  ==============================================================================
  
  Base class imitating Max 7's pfft~ object.  Override spectrumCallback as a means
  of defining the subpatch required by pfft~.  spectrumCallback will default to
  a passthrough - giving you an idea of how the windowing etc mangles a signal.
  
*/

#ifndef PFFT_H_INCLUDED
#define PFFT_H_INCLUDED

#include "JuceHeader.h"

template <typename FloatType> class PfftWindow
{
public:
    PfftWindow(const int winSize);
    ~PfftWindow();
    const FloatType operator[](const int index) { return windowData->getSample(0, index); }
    void applyTo(AudioBuffer<FloatType>& buffer);
protected:
    int size;
    ScopedPointer<AudioBuffer<FloatType>> windowData;
};

template <typename FloatType>
class LinearWindow : public PfftWindow<FloatType>
{
public:
    LinearWindow(const int winSize);
    //~LinearWindow();
};


template <typename FloatType> class Pfft
{
public:
    Pfft(const int size=1024, const int hopFac=4, const int numChannels=1);
    virtual ~Pfft();
    void setNumberOfChannels(const int numberOfChannels);
    void processBlock(AudioBuffer<FloatType> &buffer);
    
protected:
    // not happy with this, check Meyers Effective C++ Item 35
    virtual void spectrumCallback(const float *input, float *output);
    
private:
    bool isPowerOf2(const int n) { return n > 0 && !(n & (n-1)); }
    void initializeProcessBuffers();
    void processFrame(AudioBuffer<FloatType>& frame);
    void mergeFrameToOutputBuffer(const AudioBuffer<FloatType>& frame);

    ScopedPointer<FFT> fft;
    ScopedPointer<LinearWindow<FloatType>> window;
    ScopedPointer<AudioBuffer<FloatType>> processBuffer;
    ScopedPointer<AudioBuffer<FloatType>> frameBuffer;
    
    //slated for removal
    OwnedArray<AudioBuffer<FloatType>> processBuffers;
    
    ScopedPointer<AudioBuffer<FloatType>> outputBuffer;
    
    int fftSize;
    int overlapFactor;
    int hopSize;
    int numberOfAudioChannels;
    int outputBufferWriteIndex;
    int outputBufferSamplesReady;
    int outputBufferReadIndex;
    int outputBufferSize;
    int processBufferWriteIndex;
    int processBufferTriggerIndex;
    int frameBufferStartIndex;
    
    // remove?
    int *processBufferIndices;
};

namespace PfftBufferUtils {
    template <typename T>
    void ringBufferCopy(const AudioBuffer<T>& source, const int& sourceStartIndex, AudioBuffer<T>& dest, const int& destStartIndex, const int& numSamples, bool overlay = false);
}

#endif  // PFFT_H_INCLUDED
