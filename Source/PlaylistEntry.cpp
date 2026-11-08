/*
  ==============================================================================

    PlaylistEntry.cpp

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PlaylistEntry.h"

PlaylistEntry::PlaylistEntry(const juce::File& file, juce::AudioFormatManager& formatManager)
{
    audioFile = file.getFullPathName();
    fileName  = file.getFileName();
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(file));
    
    if (reader != nullptr) {
        juce::String artist = reader->metadataValues.getValue("artist", "").trim();
        artistName = artist.isNotEmpty() ? artist : "unknown";
        
        juce::String title = reader->metadataValues.getValue("title", "").trim();
        trackName = title.isNotEmpty() ? title : file.getFileNameWithoutExtension();
        
        trackDuration = reader->lengthInSamples / reader->sampleRate;
        }
    else {
        artistName = "unknown";
        trackName = file.getFileNameWithoutExtension();
        trackDuration = 0.0;
    }
}

PlaylistEntry::~PlaylistEntry() = default;

juce::String PlaylistEntry::getAudioFile() const {
     return audioFile;
 }

juce::String PlaylistEntry::getFileName() const {
     return fileName;
 }

juce::String PlaylistEntry::getArtistName() const {
     return artistName;
 }

juce::String PlaylistEntry::getTrackName() const {
     return trackName;
 }

double PlaylistEntry::getTrackDuration() const {
     return trackDuration;
 }
