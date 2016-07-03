/*
  ==============================================================================

    Pfft.cpp
    Created: 29 Jun 2016 9:24:41pm
    Author:  Oli

  ==============================================================================
*/

#include "Pfft.h"

template <typename FloatType> Pfft<FloatType>::Pfft(const int size, const int hopFac, const int numChannels)
    : fftSize(size), overlapFactor(hopFac),
    outputBufferWriteIndex(0),
    outputBufferSamplesReady(0), outputBufferReadIndex(0),
    outputBufferSize(size)
{
    // need to assert fftSize is a power of 2, and overlapFactor also, and < fftSize
    jassert(overlapFactor < fftSize && isPowerOf2(fftSize) && isPowerOf2(overlapFactor));
    fft = new FFT(fftSize, false);
    window = new LinearWindow<FloatType>(fftSize);
    
    hopSize = fftSize/overlapFactor; // both powers of 2, and fftSize > ovelapFactor
    
    setNumberOfChannels(numChannels);
    
    /*
    outputBuffer = new float[outputBufferSize];
    
    processBuffers = new float*[overlapFactor];
    processBufferIndices = new int[overlapFactor];
    for(int i=0; i<overlapFactor; i++) {
        processBuffers[i] = new float[fftSize];
    }
    
    initializeProcessBuffers();
    */
    
}

template <typename FloatType> Pfft<FloatType>::~Pfft()
{
    fft = nullptr;
    window = nullptr;
    outputBuffer = nullptr;

    /*
    for(int i=0; i<overlapFactor; i++) {
        delete[] processBuffers[i];
    }
    delete[] processBuffers;
    */
    
    processBuffers.clear();
    delete[] processBufferIndices;
    //delete[] outputBuffer;
}

template <typename T> void Pfft<T>::setNumberOfChannels(const int numberOfChannels)
{
    numberOfAudioChannels = numberOfChannels;
    initializeProcessBuffers();
}

template <typename T> void Pfft<T>::initializeProcessBuffers()
{
    processBuffers.clear();
    
    int staggeredWriteOffset = fftSize;
    for(int i=1; i<=overlapFactor; i++) {
        const int bufferBeingInitialized = i%overlapFactor;
        /*
        for(int j=0; j<fftSize; j++) {
            processBuffers[bufferBeingInitialized][j]=0;
        }
        */
        processBuffers.add(new AudioBuffer<T>(numberOfAudioChannels, fftSize));
        processBufferIndices[bufferBeingInitialized] = staggeredWriteOffset;
        staggeredWriteOffset -= hopSize;
    }
    
    outputBuffer = new AudioBuffer<T>(numberOfAudioChannels, outputBufferSize);
    /*
    for(int i=0; i<outputBufferSize; i++) {
        outputBuffer[i]=0;
    }
    */
}

template <typename T> void Pfft<T>::spectrumCallback(const float *in, float *out)
{
    for(int i=0; i<fftSize; i++) {
        out[i]=in[i];
    }
}

template <typename FloatType> void Pfft<FloatType>::processBlock(AudioBuffer<FloatType> &buffer) {
    /*
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
    */

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

template <typename FloatType> void Pfft<FloatType>::processFrame(const AudioBuffer<FloatType>& frame) {
    
    //window->applyTo(frame);
    // then fft
    // then ifft
    // then window again
    
    // re-work PfftWindow to use AudioBuffers instead ;)
    //window->applyTo(frame);
}

template <typename FloatType> void Pfft<FloatType>::mergeFrameToOutputBuffer(const AudioBuffer<FloatType>& frame) {
    // the following might be replaced by calls to addFrom and copyFrom instead of loops
    int i;
    for(i=0; i<fftSize-hopSize; i++) {
        //pushSample(frame[i], true);
        //outputBuffer->addSample(
    }
    for(; i<fftSize; i++) {
        //pushSample(frame[i]);
    }
    outputBufferSamplesReady += hopSize;
}

template <typename FloatType>
PfftWindow<FloatType>::PfftWindow(const int winSize)
    : size(winSize)
{
    windowData = new AudioBuffer<FloatType>(1, size);
}

template <typename T> PfftWindow<T>::~PfftWindow() {
    windowData = nullptr;
}


template <typename FloatType> void PfftWindow<FloatType>::applyTo(AudioBuffer<FloatType>& buffer)
{
    /*
    for(int i=0; i<size; i++) {
        buffer[i] *= windowData[i];
    }
    */
}

template <typename FloatType>
LinearWindow<FloatType>::LinearWindow(const int winSize)
    : PfftWindow<FloatType>(winSize)
{
    const int m = size/2;
    for(int i=0; i<m; i++) {
        windowData->setSample(0, i, i / static_cast<FloatType>(m));
    }
    for(int i=m; i<size; i++) {
        windowData->setSample(0, i, (size - i) / static_cast<FloatType>(m));
    }
}

// create our classes
template class Pfft<float>;
//template class PfftWindow<float>;
template class LinearWindow<float>;
