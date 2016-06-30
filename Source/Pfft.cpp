/*
  ==============================================================================

    Pfft.cpp
    Created: 29 Jun 2016 9:24:41pm
    Author:  Oli

  ==============================================================================
*/

#include "Pfft.h"

Pfft::Pfft(const int size, const int hopFac) : fftSize(size), overlapFactor(hopFac),
    preprocessBufferIndex(0)
{
    fft = new FFT(fftSize, false);
    window = new LinearWindow(fftSize);
    
    hopSize = fftSize/overlapFactor; // ! presumptuousness 1
    preprocessBufferSize = hopSize * (2*overlapFactor - 1); // ! presumtpuousness 1
    preprocessBuffer = new float[preprocessBufferSize];
    for(int i=0; i<preprocessBufferSize; i++) {
        preprocessBuffer[i]=0;
    }
}

Pfft::~Pfft()
{
    fft = nullptr;
    window = nullptr;
    delete[] preprocessBuffer;
}

void Pfft::spectrumCallback(const float *in, float *out)
{
    for(int i=0; i<fftSize; i++) {
        out[i]=in[i];
    }
}

void Pfft::processBlock(float *buffer, const int bufferSize) {
    for(int i=0; i<bufferSize; i++) {
        preprocessBuffer[preprocessBufferIndex++] = buffer[i];
        if(preprocessBufferIndex >= preprocessBufferSize) {
            preprocessBufferIndex = 0;
        }
    }
}

PfftWindow::PfftWindow(const int winSize) : size(winSize)
{
    windowData = new float[size];
}

PfftWindow::~PfftWindow() {
    delete[] windowData;
}


void PfftWindow::applyTo(float* const buffer)
{
    for(int i=0; i<size; i++) {
        buffer[i] *= windowData[i];
    }
}

LinearWindow::LinearWindow(const int winSize) : PfftWindow(winSize)
{
    const int m = size/2;
    for(int i=0; i<m; i++) {
        windowData[i] = i / static_cast<float>(m);
    }
    for(int i=m; i<size; i++) {
        windowData[i] = (size - i) / static_cast<float>(m);
    }
}