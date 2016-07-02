/*
  ==============================================================================

    Pfft.cpp
    Created: 29 Jun 2016 9:24:41pm
    Author:  Oli

  ==============================================================================
*/

#include "Pfft.h"

Pfft::Pfft(const int size, const int hopFac) : fftSize(size), overlapFactor(hopFac),
    outputBufferWriteIndex(0), outputBufferSamplesReady(0), outputBufferReadIndex(0),
    outputBufferSize(size)
{
    // need to assert fftSize is a power of 2, and overlapFactor also, and < fftSize
    jassert(overlapFactor < fftSize && isPowerOf2(fftSize) && isPowerOf2(overlapFactor));
    fft = new FFT(fftSize, false);
    window = new LinearWindow(fftSize);
    
    hopSize = fftSize/overlapFactor; // both powers of 2, and fftSize > ovelapFactor
    outputBuffer = new float[outputBufferSize];
    
    processBuffers = new float*[overlapFactor];
    processBufferIndices = new int[overlapFactor];
    for(int i=0; i<overlapFactor; i++) {
        processBuffers[i] = new float[fftSize];
    }
    
    initializeProcessBuffers();
    
}

Pfft::~Pfft()
{
    fft = nullptr;
    window = nullptr;
    for(int i=0; i<overlapFactor; i++) {
        delete[] processBuffers[i];
    }
    delete[] processBuffers;
    delete[] processBufferIndices;
    delete[] outputBuffer;
}

void Pfft::initializeProcessBuffers()
{
    int staggeredWriteOffset = fftSize;
    for(int i=1; i<=overlapFactor; i++) {
        const int bufferBeingInitialized = i%overlapFactor;
        for(int j=0; j<fftSize; j++) {
            processBuffers[bufferBeingInitialized][j]=0;
        }
        processBufferIndices[bufferBeingInitialized] = staggeredWriteOffset;
        staggeredWriteOffset -= hopSize;
    }
    for(int i=0; i<outputBufferSize; i++) {
        outputBuffer[i]=0;
    }
}

void Pfft::spectrumCallback(const float *in, float *out)
{
    for(int i=0; i<fftSize; i++) {
        out[i]=in[i];
    }
}

void Pfft::processBlock(float *buffer, const int bufferSize) {
    
    for(int i=0; i<bufferSize; i++) {
        for(int j=0; j<overlapFactor; j++) {
            float *const currentProcessBuffer = processBuffers[j];
            currentProcessBuffer[processBufferIndices[j]++]=buffer[i];
            if(processBufferIndices[j] >= fftSize) {
                processBufferIndices[j]=0;
                // ...and trigger a passthrough of the window
                processFrame(currentProcessBuffer);
                mergeFrameToOutputBuffer(currentProcessBuffer);
            }
        }
    }
    
    if(outputBufferSamplesReady >= bufferSize) {
        for(int i=0; i<bufferSize; i++) {
            buffer[i] = outputBuffer[outputBufferReadIndex++];
            if(outputBufferReadIndex >= outputBufferSize) outputBufferReadIndex = 0;
        }
        outputBufferSamplesReady -= bufferSize;
    }

            /* 
                maybe PfftWindow::applyTo should not be in-place..?
            
                we've hit a window boundary, window our last fftSize samples
                perform fft
                perform spectrumCallback
                perform inverse fft
                window again
                overlay the first (overlapFactor - 1) * hopSize samples
                and replace remaining hopSize (up to fftSize)
                
                Need to take care of wrapping around the buffer
            
            */

    
}

void Pfft::processFrame(float *const frame) {
    
    //window->applyTo(frame);
    // then fft
    // then ifft
    // then window again
    
    window->applyTo(frame);
}

void Pfft::mergeFrameToOutputBuffer(const float *const frame) {
    int i;
    for(i=0; i<fftSize-hopSize; i++) {
        pushSample(frame[i], true);
    }
    for(; i<fftSize; i++) {
        pushSample(frame[i]);
    }
    outputBufferSamplesReady += hopSize;
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