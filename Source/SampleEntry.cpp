/*
  ==============================================================================

    SampleEntry.cpp

  ==============================================================================
*/

#include "SampleEntry.h"

SampleEntry::SampleEntry(const juce::File& file, const juce::String& _sourceName, juce::AudioFormatManager& formatManager)
    : sampleFile(file), sourceName(_sourceName)
{
    sampleName = file.getFileName();
    
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr) {
        durationInSeconds = (double) reader->lengthInSamples / reader->sampleRate;
        delete reader;
    } else {
        durationInSeconds = 0.0;
    }
}

SampleEntry::SampleEntry(const juce::String& _filePath, const juce::String& _sampleName, const juce::String& _sourceName, double _duration, double _volume)
    : sampleFile(_filePath), sampleName(_sampleName), sourceName(_sourceName), durationInSeconds(_duration), volume(_volume)
{
}
