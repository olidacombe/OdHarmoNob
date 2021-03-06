/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// this function seems impotent - although it is surely getting called
// hmmmmmm....
void freqMultiplySpectrumCBO::spectrumCallback(AudioBuffer<float>& buf)
{
    roBuf = buf;
    float **wps = buf.getArrayOfWritePointers();
    const float **rps = roBuf.getArrayOfReadPointers();
    
    const int m=size/2;
    for(int c=0; c<numChannels; c++) {
        for(int b=0; b<m; b++) {
            wps[c][b]=rps[c][static_cast<int>(b/factor) % m];
        }
    }
    //buf=roBuf;
}



//==============================================================================
OdHarmoNobAudioProcessor::OdHarmoNobAudioProcessor()
{
    //spectrumCallbackObject = new freqMultiplySpectrumCBO();
}

OdHarmoNobAudioProcessor::~OdHarmoNobAudioProcessor()
{
    pfft = nullptr;
    spectrumCallbackObject = nullptr;
}

//==============================================================================
const String OdHarmoNobAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OdHarmoNobAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OdHarmoNobAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double OdHarmoNobAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OdHarmoNobAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OdHarmoNobAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OdHarmoNobAudioProcessor::setCurrentProgram (int index)
{
}

const String OdHarmoNobAudioProcessor::getProgramName (int index)
{
    return String();
}

void OdHarmoNobAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void OdHarmoNobAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{   
    const int fftSize = 1024;
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numberOfProcessChannels = jmin(totalNumInputChannels, totalNumOutputChannels);
    spectrumCallbackObject = new freqMultiplySpectrumCBO(fftSize, numberOfProcessChannels);
    spectrumCallbackObject->setFactor(0.2);
    pfft = new OdPfft::Pfft<float>(spectrumCallbackObject, numberOfProcessChannels, fftSize, 2, samplesPerBlock);

}

void OdHarmoNobAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    pfft = nullptr;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OdHarmoNobAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
{
    // Reject any bus arrangements that are not compatible with your plugin

    const int numChannels = preferredSet.size();

   #if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
   #elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
   #else
    if (numChannels != 1 && numChannels != 2)
        return false;

    if (! AudioProcessor::setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
   #endif

    return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif

void OdHarmoNobAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int bufferSize = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, bufferSize);

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...

    pfft->processBlock(buffer);

}

//==============================================================================
bool OdHarmoNobAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* OdHarmoNobAudioProcessor::createEditor()
{
    return new OdHarmoNobAudioProcessorEditor (*this);
}

//==============================================================================
void OdHarmoNobAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void OdHarmoNobAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OdHarmoNobAudioProcessor();
}
