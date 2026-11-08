/*
  ==============================================================================

    MainComponent.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include "MusicLibrary.h"
#include "LoopSamplerPanel.h"
#include "SessionManager.h"

class MainComponent  : public juce::AudioAppComponent,
                       public juce::DragAndDropContainer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbCache{100};
    
    
    DJAudioPlayer player1{formatManager};
    DeckGUI deckGUI1{&player1, formatManager, thumbCache, 1};
    
    DJAudioPlayer player2{formatManager};
    DeckGUI deckGUI2{&player2, formatManager, thumbCache, 2};

    juce::MixerAudioSource mixerSource;
    
    MusicLibrary musicLibrary{&player1};
    LoopSamplerPanel loopSamplerPanel{formatManager};
    
    LoopRecorder masterRecorder;
    double currentSampleRate = 0;
    SessionManager sessionManager{musicLibrary.getPlaylistComponent(), &masterRecorder, currentSampleRate};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
