/*
  ==============================================================================

    MusicLibrary.cpp

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MusicLibrary.h"

MusicLibrary::MusicLibrary(DJAudioPlayer* playerToUse)
     : playlistComponent(playerToUse)
{
    addAndMakeVisible(label1);
    label1.setText("Music Library", juce::dontSendNotification);
    label1.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    
    addAndMakeVisible(loadMusicButton);
    loadMusicButton.setButtonText("Load Music");
    
    addAndMakeVisible(playlistComponent);
    loadMusicButton.addListener(this);

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    playlistComponent.loadPlaylist(formatManager);
}

MusicLibrary::~MusicLibrary() = default;

void MusicLibrary::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff181818));

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);
}

void MusicLibrary::resized()
   {
       double headerH = 32.0;
       double w = getWidth();
       
       label1.setBounds(5, 2, 160, headerH - 4);
       
       int btnW = 125;
       loadMusicButton.setBounds(w - btnW - 5, 4, btnW, headerH - 8);
       
       playlistComponent.setBounds(0, headerH, w, getHeight() - headerH);
   }

void MusicLibrary::buttonClicked (juce::Button* button) {
   if (button == &loadMusicButton) {
        fchooser = std::make_unique<juce::FileChooser> ("Select an audio file to play...",
                                                        juce::File{},
                                                        "*.wav;*.flac;*.mp3;*.aiff;*.m4a");
        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        fchooser->launchAsync (folderChooserFlags, [this] (const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file.exists()) {
                juce::AudioFormatManager formatManager;
                formatManager.registerBasicFormats();
                PlaylistEntry playlistEntry{file, formatManager};
                playlistComponent.playlistEntries.push_back(playlistEntry);
                
                playlistComponent.updateTable();
            }
        });
    }
}


