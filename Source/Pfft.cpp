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
    numberOfAudioChannels(1),
    processBufferWriteIndex(0), processBufferTriggerIndex(0),
    frameBufferStartIndex(0),
    outputBufferWriteIndex(0), outputBufferSamplesReady(0),
    outputBufferReadIndex(0), outputBufferSize(size),
    windowMergeGain(0.5)
{
    // need to assert fftSize is a power of 2, and overlapFactor also, and < fftSize
    jassert(overlapFactor < fftSize && isPowerOf2(fftSize) && isPowerOf2(overlapFactor));
    fft = new FFT(fftSize, false);
    window = new LinearWindow<FloatType>(fftSize);
    
    hopSize = fftSize/overlapFactor; // both powers of 2, and fftSize > ovelapFactor
    
    setNumberOfChannels(numChannels);

    
}

template <typename FloatType> Pfft<FloatType>::~Pfft()
{
    fft = nullptr;
    window = nullptr;
    processBuffer = nullptr;
    frameBuffer = nullptr;
    outputBuffer = nullptr;
}

template <typename T> void Pfft<T>::setNumberOfChannels(const int numberOfChannels)
{
    numberOfAudioChannels = numberOfChannels;
    initializeProcessBuffers();
}

template <typename T> void Pfft<T>::initializeProcessBuffers()
{
    processBuffer = new AudioBuffer<T>(numberOfAudioChannels, fftSize);
    processBuffer->clear();
    processBufferWriteIndex = 0;
    processBufferTriggerIndex = 0;
    frameBuffer = new AudioBuffer<T>(numberOfAudioChannels, fftSize);
    frameBuffer->clear();
    frameBufferStartIndex = 0;
    outputBuffer = new AudioBuffer<T>(numberOfAudioChannels, outputBufferSize);
    outputBuffer->clear();
    outputBufferWriteIndex = 0;
    outputBufferSamplesReady = 0;
    outputBufferReadIndex = 0;
}

template <typename T> void Pfft<T>::spectrumCallback(const float *in, float *out)
{
    for(int i=0; i<fftSize; i++) {
        out[i]=in[i];
    }
}

template <typename FloatType> void Pfft<FloatType>::processBlock(AudioBuffer<FloatType> &buffer) {
    
    const int bufferSize = buffer.getNumSamples();
    int remainingSamplesToProcess = bufferSize;
    int bufferReadIndex = 0;
    
    while(remainingSamplesToProcess > 0) {
        const int samplesToProcess = jmin(processBufferTriggerIndex, remainingSamplesToProcess);
        
        PfftBufferUtils::ringBufferCopy(*processBuffer, processBufferWriteIndex,
            buffer, bufferReadIndex, samplesToProcess);
        
        // update indices here
        bufferReadIndex += samplesToProcess;
        processBufferWriteIndex = (processBufferWriteIndex + samplesToProcess) % fftSize;
        remainingSamplesToProcess -= samplesToProcess;
        processBufferTriggerIndex -= samplesToProcess;
        
        if(processBufferTriggerIndex <= 0) {
            processBufferTriggerIndex = hopSize; // would do += but jmin assignment of samplesToProcess should ensure ok
            frameBufferStartIndex = (frameBufferStartIndex + hopSize) % fftSize; // first flush starts at hopSize
            
            // flush through here
            PfftBufferUtils::ringBufferCopy(*frameBuffer, 0, *processBuffer, frameBufferStartIndex, fftSize);
            processFrame(*frameBuffer);
            mergeFrameToOutputBuffer(*frameBuffer);
            
            outputBufferSamplesReady += hopSize;
        }

    }
    
    if(outputBufferSamplesReady >= bufferSize) {
        PfftBufferUtils::ringBufferCopy(buffer, 0, *outputBuffer, outputBufferReadIndex, bufferSize);
        outputBufferReadIndex = (outputBufferReadIndex + bufferSize) % outputBufferSize;
        outputBufferSamplesReady -= bufferSize;
    } else { // clear buffer
        buffer.clear();
    }
 
}

template <typename FloatType> void Pfft<FloatType>::processFrame(AudioBuffer<FloatType>& frame) {
    
    //window->applyTo(frame);
    // then fft
    // then ifft
    // then window again
    
    // re-work PfftWindow to use AudioBuffers instead ;)
    window->applyTo(frame);
}

template <typename FloatType> void Pfft<FloatType>::mergeFrameToOutputBuffer(const AudioBuffer<FloatType>& frame) {
    PfftBufferUtils::ringBufferCopy(*outputBuffer, outputBufferWriteIndex, frame, 0, fftSize - hopSize, true, windowMergeGain);
    outputBufferWriteIndex = (outputBufferWriteIndex + fftSize - hopSize) % outputBufferSize;
    PfftBufferUtils::ringBufferCopy( *outputBuffer, outputBufferWriteIndex, frame, fftSize - hopSize, hopSize, false, windowMergeGain);
    outputBufferWriteIndex = (outputBufferWriteIndex + hopSize) % outputBufferSize;
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
    jassert(buffer.getNumSamples() == this->size);
    const int numChannels = buffer.getNumChannels();
    FloatType **bufferPointers = buffer.getArrayOfWritePointers();
    const FloatType *windowPointer = windowData->getReadPointer(0);
    
    for(int c=0; c<numChannels; c++) {
        for(int i=0; i<this->size; i++) {
            bufferPointers[c][i] *= windowPointer[i];
        }
    }
}

template <typename FloatType>
LinearWindow<FloatType>::LinearWindow(const int winSize)
    : PfftWindow<FloatType>(winSize)
{
    std::cout << "instantiating LinearWindow" << std::endl;
    const int m = this->size/2;
    for(int i=0; i<m; i++) {
        this->windowData->setSample(0, i, i / static_cast<FloatType>(m));
    }
    for(int i=m; i<this->size; i++) {
        this->windowData->setSample(0, i, (this->size - i) / static_cast<FloatType>(m));
    }
    for(int i=0; i<this->size; i++) {
        std::cout << this->windowData->getSample(0, i) << std::endl;
    }
}


template <typename T>
void PfftBufferUtils::ringBufferCopy(AudioBuffer<T>& dest, const int& destStartIndex, const AudioBuffer<T>& source, const int& sourceStartIndex, const int& numSamples, bool overlay, const T& gain)
{
    const int sourceBufferSize = source.getNumSamples();
    const int destBufferSize = dest.getNumSamples();
    
    // a bit heavy
    const int numSourceChannels = source.getNumChannels();
    const int numDestChannels = dest.getNumChannels();
    const int numChannels = jmin(numDestChannels, numSourceChannels);
    
    
    
    void (*copyFunc)(AudioBuffer<T>&, int, int, const AudioBuffer<T>&, int, int, int, T);
    copyFunc = overlay ? &PfftBufferUtils::audioBufferCopyOverlayWrapper<T> :
                         &PfftBufferUtils::audioBufferCopyOverwriteWrapper<T>;
    
    /* // a bit heavy
    const int numSamplesToCopy = jmin(numSamples, destBufferSize);
    if(numSamplesToCopy < numSamples) {
        sourceStartIndex = (sourceStartIndex + numSamples - numSamplesToCopy) % sourceBufferSize;
        destStartIndex = (destStartIndex + numSamples - numSamplesToCopy) % destBufferSize;
    }
    */
    
    // discard excess instead ?
    // or just forge ahead writing in a ring and let the implementor pay the price
    
    int remainingSamplesToCopy = numSamples;
    int readIndex = sourceStartIndex % sourceBufferSize;
    int writeIndex = destStartIndex % destBufferSize;
    
    while(remainingSamplesToCopy > 0) {
        const int samplesToCopy = jmin(remainingSamplesToCopy, destBufferSize - writeIndex, sourceBufferSize - readIndex);
        
        // use function pointer to select overlay or not
        for(int c=0; c<numChannels; c++) {
            copyFunc(dest, c, writeIndex, source, c, readIndex, samplesToCopy, gain);
        }
        
        remainingSamplesToCopy -= samplesToCopy;
        writeIndex = (writeIndex + samplesToCopy) % destBufferSize;
        readIndex = (readIndex + samplesToCopy) % sourceBufferSize;
    }
    
}

// explicitly instantiate templated classes
template class Pfft<float>;
