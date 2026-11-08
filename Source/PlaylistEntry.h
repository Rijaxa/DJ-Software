/*
  ==============================================================================

    PlaylistEntry.h
 
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class PlaylistEntry
{
public:
    PlaylistEntry(const juce::File& file, juce::AudioFormatManager& formatManager);
    ~PlaylistEntry();
    juce::String getAudioFile() const;
    juce::String getFileName() const;
    juce::String getArtistName() const;
    juce::String getTrackName() const;
    double getTrackDuration() const;

private:
    juce::String audioFile;
    juce::String fileName;
    juce::String artistName;
    juce::String trackName;
    double trackDuration;
};
