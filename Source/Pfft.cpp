/*
  ==============================================================================

    Pfft.cpp
    Created: 29 Jun 2016 9:24:41pm
    Author:  Oli

  ==============================================================================
*/

#include "Pfft.h"

Pfft::Pfft(const int size, const int hopFac) : fftSize(size), overlapFactor(hopFac)
{
    fft = new FFT(fftSize, false);
}

Pfft::~Pfft()
{
    fft = nullptr;
}

void Pfft::spectrumCallback(const float *in, float *out)
{
    for(int i=0; i<fftSize; i++) {
        out[i]=in[i];
    }
}

void processBlock(float *buffer, const int bufferSize) {
    
}