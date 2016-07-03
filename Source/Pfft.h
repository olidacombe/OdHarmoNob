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

class PfftWindow
{
public:
    PfftWindow(const int winSize);
    ~PfftWindow();
    const int getSize() { return size; }
    const float operator[](const int index) { return windowData[index]; }
    void applyTo(float* const buffer);
protected:
    int size;
    float *windowData;
};

class LinearWindow : public PfftWindow
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
    void processFrame(float *const frame);
    void mergeFrameToOutputBuffer(const float *const frame);

    ScopedPointer<FFT> fft;
    ScopedPointer<LinearWindow> window;
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
    int *processBufferIndices;
};



#endif  // PFFT_H_INCLUDED
