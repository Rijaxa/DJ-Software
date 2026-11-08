/*
  ==============================================================================

    SampleEntry.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SampleEntry {
public:
    SampleEntry() = default;
    
    SampleEntry(const juce::File& file, const juce::String& _sourceName, juce::AudioFormatManager& formatManager);

    SampleEntry(const juce::String& _filePath, const juce::String& _sampleName, const juce::String& _sourceName, double _duration, double _volume = 1.0);

    juce::File getSampleFile() const { return sampleFile; }
    
    juce::String getSampleName() const { return sampleName; }

    juce::String getSourceName() const { return sourceName; }
    
    double getDuration() const { return durationInSeconds; }
    double getVolume() const { return volume; }
    void setVolume(double vol) { volume = vol; }

private:
    juce::File sampleFile;
    juce::String sampleName;
    juce::String sourceName;
    double durationInSeconds{0.0};
    double volume{1.0};
};
