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
    processBufferWriteIndex(0), processBufferTriggerIndex(0),
    frameBufferStartIndex(0),
    outputBufferWriteIndex(0), outputBufferSamplesReady(0),
    outputBufferReadIndex(0), outputBufferSize(size)
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
    processBuffer = nullptr;
    frameBuffer = nullptr;
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
    
    processBuffer = new AudioBuffer<T>(numberOfAudioChannels, fftSize);
    processBufferWriteIndex = 0;
    processBufferTriggerIndex = 0;
    frameBuffer = new AudioBuffer<T>(numberOfAudioChannels, fftSize);
    frameBufferStartIndex = 0;
    outputBuffer = new AudioBuffer<T>(numberOfAudioChannels, outputBufferSize);
    outputBufferWriteIndex = 0;
    outputBufferSamplesReady = 0;
    outputBufferReadIndex = 0;
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
    
    int remainingSamplesToProcess = buffer.getNumSamples();
    int bufferReadIndex = 0;
    
    while(remainingSamplesToProcess > 0) {
        const int samplesToProcess = jmin(processBufferTriggerIndex, remainingSamplesToProcess);
        const int samplesToPushToEnd = jmin(outputBufferSize - outputBufferWriteIndex, samplesToProcess);
        const int samplesToPushToBeginning = jmax(samplesToProcess - samplesToPushToEnd, 0);
        
        // do the writes here
        // samplesToPushToEnd should always be > 0
        for(int c=0; c<numberOfAudioChannels; c++) {
            processBuffer->copyFrom(c, outputBufferWriteIndex, buffer, c, bufferReadIndex, samplesToPushToEnd);
            bufferReadIndex += samplesToPushToEnd;
        }
        if(samplesToPushToBeginning > 0) {
            for(int c=0; c<numberOfAudioChannels; c++) {
                processBuffer->copyFrom(c, 0, buffer, c, bufferReadIndex, samplesToPushToBeginning);
                bufferReadIndex += samplesToPushToBeginning;
            }
        }
        
        
        // update indices here
        processBufferWriteIndex = (processBufferWriteIndex + samplesToProcess) % fftSize;
        
        
        remainingSamplesToProcess -= samplesToProcess;
        
        if(processBufferTriggerIndex <= 0) {
            processBufferTriggerIndex = hopSize;
            frameBufferStartIndex = (frameBufferStartIndex + hopSize) % fftSize; // first flush starts at hopSize
            
            // flush through here
            
            outputBufferSamplesReady += hopSize;
        }
        
        
        
        /*
        // tiredd... 
        int numSamplesToPushToProcessBuffer = 
        
        remainingSamplesToProcess -= numSamplesToPushToProcessBuffer;
        // or
        remainingSamplesToProcess -= hopSize;
        */
    }
    
    /*
    
    could do with a ProcessBuffers class which can be ordered!
    give them functions for setting write pointer position
    which update remaining samples to fill tally
    then order by that tally.
    
    Give them a drinkFrom function.... how do we keep track of
    1) whether they're full
    2) what the read index now is
    
    ... or maybe just do a buffer copy thing
    
    */
    
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

template <typename FloatType> void Pfft<FloatType>::processFrame(AudioBuffer<FloatType>& frame) {
    
    //window->applyTo(frame);
    // then fft
    // then ifft
    // then window again
    
    // re-work PfftWindow to use AudioBuffers instead ;)
    window->applyTo(frame);
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
    const int m = this->size/2;
    for(int i=0; i<m; i++) {
        this->windowData->setSample(0, i, i / static_cast<FloatType>(m));
    }
    for(int i=m; i<this->size; i++) {
        this->windowData->setSample(0, i, (this->size - i) / static_cast<FloatType>(m));
    }
}

template <typename T>
void PfftBufferUtils::ringBufferCopy(const AudioBuffer<T>& source, const int& sourceStartIndex, AudioBuffer<T>& dest, const int& destStartIndex, const int& numSamples, bool overlay)
{
    const int sourceBufferSize = source.getNumSamples();
    const int destBufferSize = dest.getNumSamples();
    
    // a bit heavy
    const int numSourceChannels = source.getNumChannels();
    const int numDestChannels = dest.getNumChannels();
    const int numChannels = jmin(numDestChannels, numSourceChannels);

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
            dest.copyFrom(c, writeIndex, source, c, readIndex, samplesToCopy);
        }
        
        remainingSamplesToCopy -= samplesToCopy;
        writeIndex = (writeIndex + samplesToCopy) % destBufferSize;
        readIndex = (readIndex + samplesToCopy) % sourceBufferSize;
    }
    
    
}


// create our classes
template class Pfft<float>;
