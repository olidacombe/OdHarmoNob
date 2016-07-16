/*
  ==============================================================================

    OdFftwUtils.cpp
    Created: 13 Jul 2016 12:43:33pm
    Author:  Oli

  ==============================================================================
*/

#include "OdFftwUtils.h"
#include <fftw3.h>

// this will only work with double for now - needs to match type though :S, probably want to construct
// FrameBuffer using the same address
template <typename FloatType> Od1dRealFftw<FloatType>::Od1dRealFftw(const int fftSize) {
    N = fftSize;
    timeDomainData = (FloatType*) fftw_malloc(sizeof(FloatType) * N);
    // not like this for halfcomplex
    //frequencyDomainData = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    frequencyDomainData = (FloatType*) fftw_malloc(sizeof(FloatType) * N);
    forwardPlan = fftw_plan_r2r_1d(N, timeDomainData, frequencyDomainData, FFTW_R2HC, FFTW_MEASURE);
    inversePlan = fftw_plan_r2r_1d(N, frequencyDomainData, timeDomainData, FFTW_HC2R, FFTW_MEASURE);
}

template <typename FloatType> Od1dRealFftw<FloatType>::~Od1dRealFftw() {
    fftw_destroy_plan(forwardPlan);
    fftw_destroy_plan(inversePlan);
    fftw_free(timeDomainData);
    fftw_free(frequencyDomainData);
}

template <typename T> T* Od1dRealFftw<T>::getTimeDomainWritePointer() {
    return timeDomainData;
}

template <typename T> T* Od1dRealFftw<T>::getFrequencyDomainWritePointer() {
    return frequencyDomainData;
}

template class Od1dRealFftw<double>;