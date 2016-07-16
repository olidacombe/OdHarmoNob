/*
  ==============================================================================

    OdFftwUtils.h
    Created: 13 Jul 2016 12:43:33pm
    Author:  Oli

  ==============================================================================
*/

#ifndef ODFFTWUTILS_H_INCLUDED
#define ODFFTWUTILS_H_INCLUDED

#include <fftw3.h>

template <typename FloatType>
class Od1dRealFftw {
public:
    Od1dRealFftw(const int fftSize);
    ~Od1dRealFftw();
    FloatType* getTimeDomainWritePointer();
    FloatType* getFrequencyDomainWritePointer();
private:
    int N;
    FloatType *timeDomainData;
    FloatType *frequencyDomainData;
    //fftw_complex *frequencyDomainData;
    fftw_plan forwardPlan, inversePlan;
};

#endif  // ODFFTWUTILS_H_INCLUDED
