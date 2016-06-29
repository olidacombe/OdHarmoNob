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

#include "../JuceLibraryCode/JuceHeader.h"

class Pfft
{
public:
    Pfft(const int size=1024, const int hopFac=4);
    virtual ~Pfft();
    void processBlock(float *buffer, const int bufferSize);
    
protected:
    virtual void spectrumCallback(const float *input, float *output);
    
private:
    ScopedPointer<FFT> fft;
    int fftSize;
    int overlapFactor;
};


#endif  // PFFT_H_INCLUDED
