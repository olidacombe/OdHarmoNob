/*
  ==============================================================================

    OdFftwUtils.cpp
    Created: 13 Jul 2016 12:43:33pm
    Author:  Oli

  ==============================================================================
*/

#include "OdFftwUtils.h"
#include <fftw3.h>

namespace OdFftwUtils {

template <typename FloatType> Od1dRealFftw<FloatType>::Od1dRealFftw(const int fftSize, const int numChannels)
: N(fftSize), numberOfChannels(numChannels)
{
    inputArrays = new FloatType*[numberOfChannels];
    outputArrays = new FloatType*[numberOfChannels];
    for(int c=0; c<numberOfChannels; c++) {
        Od1dRealFftwPlan<FloatType> *newPlan = new Od1dRealFftwPlan<FloatType>(N);
        fftwPlans.add(newPlan);
        inputArrays[c] = newPlan->getTimeDomainWritePointer();
        outputArrays[c] = newPlan->getFrequencyDomainWritePointer();
    }
    
    audioBuffer = new AudioBuffer<FloatType>(inputArrays, numberOfChannels, N);
    frequencyBuffer = new AudioBuffer<FloatType>(outputArrays, numberOfChannels, N);
}
    
 
template <typename FloatType> Od1dRealFftw<FloatType>::~Od1dRealFftw()
{
    audioBuffer = nullptr;
    frequencyBuffer = nullptr;
    fftwPlans.clear();
    delete[] inputArrays;
    delete[] outputArrays;
}

template <typename FloatType> void Od1dRealFftw<FloatType>::forwardTransform()
{
    for(int c=0; c<numberOfChannels; c++) {
        fftwPlans[c]->forwardTransform();
    }
}

template <typename FloatType> void Od1dRealFftw<FloatType>::inverseTransform()
{
    for(int c=0; c<numberOfChannels; c++) {
        fftwPlans[c]->inverseTransform();
    }
}

template class Od1dRealFftw<float>;

}