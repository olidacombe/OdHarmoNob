/*
  ==============================================================================

    Pfft.h
    Created: 29 Jun 2016 9:24:41pm
    Author:  Oli

  ==============================================================================
  
  Base class imitating Max 7's pfft~ object.  Override spectrumCallback as a means
  of defining the subpatch required by pfft~.  spectrumCallback will default to
  a passthrough - giving you an idea of how the windowing etc mangles a signal.
  
*/

#ifndef PFFT_H_INCLUDED
#define PFFT_H_INCLUDED

#include "JuceHeader.h"
#include "OdFftwUtils.h"

namespace OdPfft {

template <typename FloatType> class PfftWindow
{
public:
    PfftWindow(const int winSize);
    ~PfftWindow();
    const FloatType operator[](const int index) { return windowData->getSample(0, index); }
    void applyTo(AudioBuffer<FloatType>& buffer);
protected:
    int size;
    ScopedPointer<AudioBuffer<FloatType>> windowData;
};

template <typename FloatType>
class LinearWindow : public PfftWindow<FloatType>
{
public:
    LinearWindow(const int winSize);
    //~LinearWindow();
};

template <typename T>
class WelchWindow : public PfftWindow<T>
{
public:
    WelchWindow(const int winSize);
};

/*
template <typename FloatType>
void defaultSpectrumCallback(AudioBuffer<FloatType>&) {};
*/

template <typename FloatType> class Pfft
{
public:
    typedef void (*frequencyDomainCallback)(AudioBuffer<FloatType>&);
    void defaultSpectrumCallback(AudioBuffer<FloatType>& buf) {};
    
    class frequencyDomainCallbackObject
    {
        public:
            virtual ~frequencyDomainCallbackObject() {};
            virtual void spectrumCallback(AudioBuffer<FloatType>&) = 0;
    };
    
    class defaultFrequencyDomainCallbackObject : public frequencyDomainCallbackObject
    {
        public:
            defaultFrequencyDomainCallbackObject(frequencyDomainCallback cb) : callback(cb) {};
            void spectrumCallback(AudioBuffer<FloatType>& buf) override {};
        private:
            frequencyDomainCallback callback;
    };
    
    //Pfft(frequencyDomainCallbackObject callbackObject, const int numChannels=1, const int size=1024, const int hopFac=4);
    Pfft(frequencyDomainCallbackObject* callbackObject, const int numChannels=1, const int size=1006, const int hopFac=4, const int blockSize=512);
    Pfft(frequencyDomainCallback cb, const int numChannels=1, const int size=1024, const int hopFac=4, const int blockSize=512);
    //Pfft(frequencyDomainCallback cb, const int numChannels, const int size, const int hopFac, const int blockSize);
    virtual ~Pfft();
    void setNumberOfChannels(const int numberOfChannels);
    void setInputBlockSize(const int blockSize);
    void processBlock(AudioBuffer<FloatType> &buffer);
    
protected:
    // not happy with this, check Meyers Effective C++ Item 35
    
    
private:
    bool isPowerOf2(const int n) { return n > 0 && !(n & (n-1)); }
    void initializeProcessBuffers();
    void initializeOutputBuffer();
    void processFrame(AudioBuffer<FloatType>& frame);
    void mergeFrameToOutputBuffer(const AudioBuffer<FloatType>& frame);
    FloatType calculateWindowMergeGain();

    //frequencyDomainCallback spectrumCallback;
    //ScopedPointer<frequencyDomainCallbackObject> spectrumCallbackObject;
    frequencyDomainCallbackObject *spectrumCallbackObject;
    
    ScopedPointer<OdFftwUtils::Od1dRealFftw<FloatType>> fftw;
    ScopedPointer<PfftWindow<FloatType>> window; // our "window function" buffer to multiply frames by in and out
    ScopedPointer<AudioBuffer<FloatType>> processBuffer; // stock up samples required for processing in windows here
    AudioBuffer<FloatType> *frameBuffer, *spectrumBuffer; // these are what we copy from processBuffer and apply callback to
    ScopedPointer<AudioBuffer<FloatType>> outputBuffer; // stock up ouput of processing here for dumps
    
    int fftSize;
    int overlapFactor;
    int hopSize;
    int numberOfAudioChannels;
    int inputBlockSize;
    int outputBufferWriteIndex;
    int outputBufferSamplesReady;
    int outputBufferReadIndex;
    int outputBufferSize;
    int processBufferWriteIndex;
    int processBufferTriggerIndex;
    int frameBufferStartIndex;
    
    FloatType windowMergeGain;

};

namespace PfftBufferUtils {
    
    template <typename T>
    void ringBufferCopy(AudioBuffer<T>& dest, const int& destStartIndex, const AudioBuffer<T>& source, const int& sourceStartIndex, const int& numSamples, bool overlay = false, const T& gain = 1.0);
    
    /*
    template <typename T>
    void audioBufferCopyOverwriteWrapper(AudioBuffer<T>& dest, int dc, int dsi, const AudioBuffer<T>& src, int sc, int ssi, int n, T gain)
    {
        //sadly can't use this for lack of gain param
        //dest.copyFrom(dc, dsi, src, sc, ssi, n);
        
        const T* readPointer = src.getReadPointer(sc);
        T* writePointer = dest.getWritePointer(dc);
        for(int i=0; i<n; i++)
        {
            writePointer[i] = readPointer[i]*gain;
        }
        
        
        
    }
    
    
    template <typename T>
    void audioBufferCopyOverlayWrapper(AudioBuffer<T>& dest, int dc, int dsi, const AudioBuffer<T>& src, int sc, int ssi, int n, T gain)
    {
        dest.addFrom(dc, dsi, src, sc, ssi, n, gain);
    }
    */
    
}

}

#endif  // PFFT_H_INCLUDED
