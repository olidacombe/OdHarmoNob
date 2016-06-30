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
    window = new LinearWindow(fftSize);
    
    buffers = new float*[overlapFactor];
    for(int i=0; i<overlapFactor; i++) {
        buffers[i] = new float[fftSize];
        for(int j=0; j<fftSize; j++) { // should ensure these initialize to 0
            buffers[i][j] = 0;
        }
    }
}

Pfft::~Pfft()
{
    fft = nullptr;
    window = nullptr;
    for(int i=0; i<overlapFactor; i++) {
        delete[] buffers[i];
    }
    delete[] buffers;
}

void Pfft::spectrumCallback(const float *in, float *out)
{
    for(int i=0; i<fftSize; i++) {
        out[i]=in[i];
    }
}

void processBlock(float *buffer, const int bufferSize) {
    
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