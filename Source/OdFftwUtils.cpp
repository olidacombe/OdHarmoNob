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

// this will only work with float for now - needs to match type though :S, probably want to construct
// FrameBuffer using the same address
template <typename FloatType> Od1dRealFftw<FloatType>::Od1dRealFftw(const int fftSize) {
    N = fftSize;

    initializeIoArrays();
    createExecutionPlans(N, timeDomainData, frequencyDomainData, FFTW_MEASURE);
    
}

template <typename FloatType> Od1dRealFftw<FloatType>::~Od1dRealFftw() {
    destroyExecutionPlans();
    destroyIoArrays();
}

template <typename T> T* Od1dRealFftw<T>::getTimeDomainWritePointer() {
    return timeDomainData;
}

template <typename T> T* Od1dRealFftw<T>::getFrequencyDomainWritePointer() {
    return frequencyDomainData;
}

//template class Od1dRealFftw<double>;
template class Od1dRealFftw<float>;

}