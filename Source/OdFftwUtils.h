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

namespace OdFftwUtils {

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
    fftwf_plan forwardPlan, inversePlan;
    
    void initializeIoArrays() {
        timeDomainData = (FloatType*) fftwf_malloc(sizeof(FloatType) * N);
        frequencyDomainData = (FloatType*) fftwf_malloc(sizeof(FloatType) * N);
    }
    void createExecutionPlans(int n, FloatType* in, FloatType *out, int flags) {
        forwardPlan = fftwf_plan_r2r_1d(n, in, out, FFTW_R2HC, flags);
        inversePlan = fftwf_plan_r2r_1d(n, out, in, FFTW_HC2R, flags);
    }
    
    void destroyIoArrays() {
        fftwf_free(timeDomainData);
        fftwf_free(frequencyDomainData);
    }
    
    void destroyExecutionPlans() {
        fftwf_destroy_plan(forwardPlan);
        fftwf_destroy_plan(inversePlan);
    }

};

// this is a bit of a ballache
/*
template<>
class Od1dRealFftw<float> {
public:


private:
    fftwf_plan forwardPlan, inversePlan;
    
    void initializeFftwFunctionPointers() {
        fftwLibMalloc = &fftwf_malloc;
    }
};


template<>
class Od1dRealFftw<double> {
public:
    void initializeIoArrays() {
        timeDomainData = (double*) fftw_malloc(sizeof(FloatType) * N);
        frequencyDomainData = (double*) fftw_malloc(sizeof(FloatType) * N);
    }
    void createExecutionPlans(int n, double* in, double *out, int flags) {
        forwardPlan = fftw_plan_r2r_1d(n, in, out, FFTW_R2HC, flags);
        inversePlan = fftw_plan_r2r_1d(n, out, in, FFTW_HC2R, flags);
    }
private:
    fftw_plan forwardPlan, inversePlan;
    
    void initializeFftwFunctionPointers() {
        fftwLibMalloc = &fftwf_malloc;
    }
};
*/


}
#endif  // ODFFTWUTILS_H_INCLUDED
