/*
  ==============================================================================

    MusicLibrary.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PlaylistComponent.h"
#include "DJAudioPlayer.h"

class MusicLibrary  : public juce::Component,
                      public juce::Button::Listener
{
public:

    MusicLibrary(DJAudioPlayer* playerToUse);
    ~MusicLibrary() override;

    void paint (juce::Graphics& g) override;

    void resized() override;

    void buttonClicked (juce::Button* button) override;
    
    PlaylistComponent* getPlaylistComponent() { return &playlistComponent; }

private:
    juce::Label label1;
    juce::TextButton loadMusicButton;
    PlaylistComponent playlistComponent;
    
    std::unique_ptr<juce::FileChooser> fchooser;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MusicLibrary)
};
