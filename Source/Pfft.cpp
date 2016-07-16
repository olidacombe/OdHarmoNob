/*
  ==============================================================================

    Pfft.cpp
    Created: 29 Jun 2016 9:24:41pm
    Author:  Oli

  ==============================================================================
*/

#include "Pfft.h"
#include <boost/math/common_factor.hpp>

using namespace OdPfft;

template <typename FloatType> Pfft<FloatType>::Pfft(const int size, const int hopFac, const int numChannels, frequencyDomainCallback callback)
    : fftSize(size), overlapFactor(hopFac),
    numberOfAudioChannels(numChannels), inputBlockSize(fftSize),
    processBufferWriteIndex(0), processBufferTriggerIndex(0),
    frameBufferStartIndex(0),
    outputBufferWriteIndex(0), outputBufferSamplesReady(0),
    outputBufferReadIndex(0), outputBufferSize(size),
    windowMergeGain(0.0), spectrumCallback(callback)
{
    // need to assert fftSize is a power of 2, and overlapFactor also, and < fftSize
    jassert(overlapFactor < fftSize && isPowerOf2(fftSize) && isPowerOf2(overlapFactor));
    
    const int fftOrder = log2(fftSize);
    hopSize = fftSize/overlapFactor; // both powers of 2, and fftSize > ovelapFactor

    //window = new LinearWindow<FloatType>(fftSize);
    window = new WelchWindow<FloatType>(fftSize);
    windowMergeGain = calculateWindowMergeGain();
    
    fftw = new OdFftwUtils::Od1dRealFftw<FloatType>(fftSize, numberOfAudioChannels);

    
    setNumberOfChannels(numChannels);

}

template <typename FloatType> Pfft<FloatType>::Pfft(const int size, const int hopFac, const int numChannels, frequencyDomainCallback callback, const int blockSize)
    : Pfft(size, hopFac, numChannels, callback)
{
    setInputBlockSize(blockSize);
}

template <typename FloatType> Pfft<FloatType>::~Pfft()
{
    fftw = nullptr;
    window = nullptr;
    processBuffer = nullptr;
    outputBuffer = nullptr;
}

template <typename T> void Pfft<T>::setNumberOfChannels(const int numberOfChannels)
{
    numberOfAudioChannels = numberOfChannels;
    initializeProcessBuffers();
}

template <typename T> void Pfft<T>::setInputBlockSize(const int blockSize)
{
    inputBlockSize = blockSize;

    // I didn't properly figure this out, but this is based on intuition and seems to work.
    // Would prefer really to sort it out properly.
    outputBufferSize = jmax( boost::math::lcm(blockSize, hopSize), fftSize) + hopSize;
    // The added hopSize is to prevent the "pure overwrite" from wrapping over past readIndex

    std::cout << "outputBufferSize: "; std::cout << outputBufferSize << std::endl;
    initializeOutputBuffer();
}

template <typename T> T Pfft<T>::calculateWindowMergeGain()
{
    AudioBuffer<T> calcBuffer(1, fftSize + hopSize);
    AudioBuffer<T> winBuffer(1, fftSize);
    calcBuffer.clear();
    for(int i=0; i<fftSize; i++) {
        winBuffer.setSample(0, i, 1.0);
    }
    window->applyTo(winBuffer);
    int winStartSample=0;
    for(int i=0; i<overlapFactor; i++) {
        PfftBufferUtils::ringBufferCopy(calcBuffer, winStartSample, winBuffer, 0, fftSize, true);
        winStartSample+=hopSize;
    }
    
    T magnitude = calcBuffer.getMagnitude(0, fftSize);
    if(magnitude <= 0.0) {
        return 0.0;
    }
    return 1.0/magnitude;
}

template <typename T> void Pfft<T>::initializeOutputBuffer()
{
    outputBuffer = new AudioBuffer<T>(numberOfAudioChannels, outputBufferSize);
    outputBuffer->clear();
    outputBufferWriteIndex = 0;
    outputBufferSamplesReady = 0;
    outputBufferReadIndex = 0;
}

template <typename T> void Pfft<T>::initializeProcessBuffers()
{
    processBuffer = new AudioBuffer<T>(numberOfAudioChannels, fftSize);
    processBuffer->clear();
    processBufferWriteIndex = 0;
    processBufferTriggerIndex = hopSize;
    //frameBuffer = new AudioBuffer<T>(numberOfAudioChannels, 2*fftSize);
    frameBuffer = fftw->getAudioBuffer();
    frameBuffer->clear();
    frameBufferStartIndex = 0;
    
    spectrumBuffer = fftw->getFrequencyBuffer();
    
    initializeOutputBuffer();
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
            processBufferTriggerIndex += hopSize; // could do = as jmin assignment of samplesToProcess should ensure ok
            frameBufferStartIndex = (frameBufferStartIndex + hopSize) % fftSize; // first flush starts at hopSize
            
            // flush through here
            PfftBufferUtils::ringBufferCopy(*frameBuffer, 0, *processBuffer, frameBufferStartIndex, fftSize);
            processFrame(*frameBuffer);
            mergeFrameToOutputBuffer(*frameBuffer);
        }

    }
    
    if(outputBufferSamplesReady >= bufferSize) {
        //some mad debug
        //std::cout << "wi, ri, sr = "; std::cout << outputBufferWriteIndex; std::cout << " "; std::cout << outputBufferReadIndex; std::cout << " "; std::cout << outputBufferSamplesReady << std::endl;
        
        PfftBufferUtils::ringBufferCopy(buffer, 0, *outputBuffer, outputBufferReadIndex, bufferSize);
        outputBufferReadIndex = (outputBufferReadIndex + bufferSize) % outputBufferSize;
        outputBufferSamplesReady -= bufferSize;
    } else { // clear buffer
        std::cout << "insufficient output samples" << std::endl;
        buffer.clear();
    }
 
}

template <typename FloatType> void Pfft<FloatType>::processFrame(AudioBuffer<FloatType>& frame) {
    //const int numChannels = frame.getNumChannels();
    //FloatType **framePointers = frame.getArrayOfWritePointers();
    
    window->applyTo(frame);

    fftw->forwardTransform();
    // do frequency domain stuff
    
    spectrumCallback(*spectrumBuffer);
    
    // then inverse
    fftw->inverseTransform();
    // then scale down by factor of fftSize - as the DFT is unnormalized
    frame.applyGain(static_cast<FloatType>(1)/fftSize);
    // then window again
    window->applyTo(frame);
    
}

template <typename FloatType> void Pfft<FloatType>::mergeFrameToOutputBuffer(const AudioBuffer<FloatType>& frame) {
    PfftBufferUtils::ringBufferCopy(*outputBuffer, outputBufferWriteIndex, frame, 0, fftSize - hopSize, true, windowMergeGain);
    outputBufferWriteIndex = (outputBufferWriteIndex + fftSize - hopSize) % outputBufferSize;
    PfftBufferUtils::ringBufferCopy( *outputBuffer, outputBufferWriteIndex, frame, fftSize - hopSize, hopSize, false, windowMergeGain);
    //outputBufferWriteIndex = (outputBufferWriteIndex + hopSize) % outputBufferSize; // No!
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
    //jassert(buffer.getNumSamples() == this->size);
    // sometimes want to window start of buffer (may delete when using fftw instead of juce fft)
    jassert(buffer.getNumSamples() >= this->size);
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
}

template <typename T>
WelchWindow<T>::WelchWindow(const int winSize)
    : PfftWindow<T>(winSize)
{
    const T m = this->size/static_cast<T>(2);
    const T s = static_cast<T>(this->size);
    T* data = this->windowData->getWritePointer(0);
    for(int i=0; i<s; i++)
    {
        data[i] = 1 - pow(((i-m)/s),2);
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
    
    
    /*
    void (*copyFunc)(AudioBuffer<T>&, int, int, const AudioBuffer<T>&, int, int, int, T);
    copyFunc = overlay ? &PfftBufferUtils::audioBufferCopyOverlayWrapper<T> :
                         &PfftBufferUtils::audioBufferCopyOverwriteWrapper<T>;
    */
    
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
        
        // benchmark this against function pointer approach (commented out)
        if(!overlay) {
            dest.clear(writeIndex, samplesToCopy);
        }
        // use function pointer to select overlay or not
        for(int c=0; c<numChannels; c++) {
            //copyFunc(dest, c, writeIndex, source, c, readIndex, samplesToCopy, gain);
            dest.addFrom(c, writeIndex, source, c, readIndex, samplesToCopy, gain);
        }
        
        remainingSamplesToCopy -= samplesToCopy;
        writeIndex = (writeIndex + samplesToCopy) % destBufferSize;
        readIndex = (readIndex + samplesToCopy) % sourceBufferSize;
    }
    
}

// explicitly instantiate templated classes
template class OdPfft::Pfft<float>;
