/*
  ==============================================================================

    SessionManager.cpp

  ==============================================================================
*/

#include "SessionManager.h"

SessionManager::SessionManager(PlaylistComponent* playlist, LoopRecorder* recorder, double& mainSampleRate)
    : playlistComponent(playlist), masterRecorder(recorder), currentSampleRate(mainSampleRate)
{
    addAndMakeVisible(recordButton);
    addAndMakeVisible(exportPlaylistButton);
    addAndMakeVisible(importPlaylistButton);
    
    recordButton.addListener(this);
    exportPlaylistButton.addListener(this);
    importPlaylistButton.addListener(this);
    
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
    exportPlaylistButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
    importPlaylistButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
}

SessionManager::~SessionManager()
{
    stopTimer();
}

void SessionManager::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff121212));
    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds(), 1);
}

void SessionManager::resized()
{
    int btnW = 200;
    int btnH = 30;
    int spacing = 10;
    int totalW = (btnW * 3) + (spacing * 2);
    
    int startX = (getWidth() - totalW) / 2;
    int startY = (getHeight() - btnH) / 2;
    
    recordButton.setBounds(startX, startY, btnW, btnH);
    exportPlaylistButton.setBounds(startX + btnW + spacing, startY, btnW, btnH);
    importPlaylistButton.setBounds(startX + (btnW * 2) + (spacing * 2), startY, btnW, btnH);
}

void SessionManager::buttonClicked (juce::Button* button)
{
    if (button == &recordButton)
    {
        if (!isRecording)
        {
            fchooser = std::make_unique<juce::FileChooser>("Save Mix As...", juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("MyLiveMix.wav"), "*.wav");
            
            fchooser->launchAsync(juce::FileBrowserComponent::saveMode,
                                  [this](const juce::FileChooser& chooser)
            {
                juce::File result = chooser.getResult();
                if (result != juce::File{})
                {
                    if (currentSampleRate > 0)
                    {
                        masterRecorder->startRecording(result, currentSampleRate);
                        isRecording = true;
                        recordButton.setButtonText("STOP RECORDING");
                        recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe74c3c));
                        startTimer(500);
                    }
                }
            });
        }
        else
        {
            masterRecorder->stopRecording();
            isRecording = false;
            recordButton.setButtonText("RECORD SET");
            recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
            stopTimer();
        }
    }
    else if (button == &exportPlaylistButton)
    {
        if (playlistComponent == nullptr) return;
        
        fchooser = std::make_unique<juce::FileChooser>("Export Playlist...", juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("MyPlaylist.m3u"), "*.m3u");
        fchooser->launchAsync(juce::FileBrowserComponent::saveMode,
                              [this](const juce::FileChooser& chooser)
        {
            juce::File result = chooser.getResult();
            if (result != juce::File{})
            {
                juce::String content;
                for (auto& entry : playlistComponent->playlistEntries)
                {
                    content += entry.getAudioFile() + "\n";
                }
                result.replaceWithText(content);
            }
        });
    }
    else if (button == &importPlaylistButton)
    {
        if (playlistComponent == nullptr) return;
        
        fchooser = std::make_unique<juce::FileChooser>("Import Playlist...", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.m3u;*.txt");
        fchooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this](const juce::FileChooser& chooser)
        {
            juce::File result = chooser.getResult();
            if (result.existsAsFile())
            {
                juce::StringArray lines;
                result.readLines(lines);
                
                juce::AudioFormatManager formatManager;
                formatManager.registerBasicFormats();
                
                for (const auto& line : lines)
                {
                    juce::File trackFile{line.trim()};
                    if (trackFile.existsAsFile())
                    {
                        PlaylistEntry entry{trackFile, formatManager};
                        playlistComponent->playlistEntries.push_back(entry);
                    }
                }
                playlistComponent->updateTable();
                playlistComponent->savePlaylist();
            }
        });
    }
}

void SessionManager::timerCallback()
{
    if (isRecording) {
        bool isOn = recordButton.getToggleState();
        recordButton.setToggleState(!isOn, juce::dontSendNotification);
        if (isOn) {
            recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffc0392b));
        } else {
            recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe74c3c));
        }
    }
}
