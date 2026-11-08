/*
  ==============================================================================

    SessionManager.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PlaylistComponent.h"
#include "LoopRecorder.h"

class SessionManager  : public juce::Component,
                        public juce::Button::Listener,
                        public juce::Timer
{
public:

    SessionManager(PlaylistComponent* playlist, LoopRecorder* recorder, double& mainSampleRate);
    
    ~SessionManager() override;

    void paint (juce::Graphics&) override;
    
    void resized() override;
    
    void buttonClicked (juce::Button* button) override;

    void timerCallback() override;

private:
    PlaylistComponent* playlistComponent;
    LoopRecorder* masterRecorder;
    double& currentSampleRate;
    
    juce::TextButton recordButton{"RECORD SET"};
    juce::TextButton exportPlaylistButton{"EXPORT PLAYLIST (.m3u)"};
    juce::TextButton importPlaylistButton{"IMPORT PLAYLIST (.m3u)"};
    
    std::unique_ptr<juce::FileChooser> fchooser;
    bool isRecording = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SessionManager)
};
