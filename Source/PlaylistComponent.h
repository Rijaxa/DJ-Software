/*
  ==============================================================================

    PlaylistComponent.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include "PlaylistEntry.h"
#include "DJAudioPlayer.h"

class PlaylistComponent  : public juce::Component,
                           public juce::TableListBoxModel,
                           public juce::Button::Listener,
                           public juce::FileDragAndDropTarget
{
public:
    std::function<void()> onStartedPlayingLocally;
    void clearHighlight() { currentlyPlayingRow = -1; tableComponent.updateContent(); }

    PlaylistComponent(DJAudioPlayer* playerToUse);
    ~PlaylistComponent() override;

    void paint (juce::Graphics&) override;
    
    void resized() override;
    
    int getNumRows () override;

    void paintRowBackground (juce::Graphics &, int rowNumber, int width, int height, bool rowIsSelected) override;
    
    void paintCell (juce::Graphics &, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    
    juce::Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, Component *existingComponentToUpdate) override;
    
    void buttonClicked(juce::Button* button) override;
    
    std::vector<PlaylistEntry> playlistEntries;
    
    void updateTable() { tableComponent.updateContent(); }
    
    juce::var getDragSourceDescription (const juce::SparseSet<int> &currentlySelectedRows) override;
    
    bool isInterestedInFileDrag (const juce::StringArray &files) override;
    void filesDropped (const juce::StringArray &files, int x, int y) override;
    
    void savePlaylist();

    void loadPlaylist(juce::AudioFormatManager& formatManager);
    
    void setPlayingFile(const juce::String& filePath, bool isbool);
    
    std::function<void(juce::File)> onLoadToDeck;
    
private:
    
    juce::TableListBox tableComponent;
    
    DJAudioPlayer* player;
    int currentlyPlayingRow = -1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistComponent)
};
