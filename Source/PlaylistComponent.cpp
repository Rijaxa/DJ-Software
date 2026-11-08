/*
  ==============================================================================

    PlaylistComponent.cpp

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PlaylistComponent.h"

PlaylistComponent::PlaylistComponent(DJAudioPlayer* playerToUse)
     : player(playerToUse)
{
    tableComponent.getHeader().addColumn("File Name",  1, 180);
    tableComponent.getHeader().addColumn("Track Name", 2, 180);
    tableComponent.getHeader().addColumn("Artist",     3, 140);
    tableComponent.getHeader().addColumn("Duration",   4, 80);
    tableComponent.getHeader().addColumn("Play",       5, 70);
    tableComponent.getHeader().addColumn("Delete",     6, 60);

    tableComponent.getHeader().setStretchToFitActive(true);

    tableComponent.setMultipleSelectionEnabled(false);
    tableComponent.setModel(this);
    addAndMakeVisible(tableComponent);
}

PlaylistComponent::~PlaylistComponent()
{
    savePlaylist();
}

void PlaylistComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff181818));

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("PlaylistComponent", getLocalBounds(),
                juce::Justification::centred, true);
}

void PlaylistComponent::resized()
{
    tableComponent.setBounds(0,0,getWidth(),getHeight());
}

int PlaylistComponent::getNumRows () {
    return (int)playlistEntries.size();
}

void PlaylistComponent::paintRowBackground (juce::Graphics & g, int rowNumber, int width, int height, bool rowIsSelected) {
    if (rowIsSelected) {
        g.fillAll(juce::Colours::orange);
    } else {
        g.fillAll(juce::Colour(0xff2d2d2d));
    }
}

void PlaylistComponent::paintCell (juce::Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= (int) playlistEntries.size())
        return;

    if (rowNumber == currentlyPlayingRow || rowIsSelected)
        g.setColour(juce::Colours::black);
    else
        g.setColour(juce::Colours::white);

    if (columnId == 1) {
        g.drawText(playlistEntries[rowNumber].getFileName(), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 2) {
        g.drawText(playlistEntries[rowNumber].getTrackName(), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 3) {
        g.drawText(playlistEntries[rowNumber].getArtistName(), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 4) {
        double seconds = playlistEntries[rowNumber].getTrackDuration();
        int mins = (int) seconds / 60;
        int secs = (int) seconds % 60;
        juce::String durationStr = juce::String::formatted("%d:%02d", mins, secs);

        g.drawText(durationStr, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
}

juce::Component* PlaylistComponent::refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected,
                                                                 juce::Component *existingComponentToUpdate)
{
    if (columnId == 5 || columnId == 6) {
        if (existingComponentToUpdate == nullptr) {
            juce::TextButton* btn = new juce::TextButton();
            btn->addListener(this);
            existingComponentToUpdate = btn;
        }
    
        auto* btn = static_cast<juce::TextButton*>(existingComponentToUpdate);
        
        if (columnId == 5) {
            btn->setComponentID("play_" + juce::String(rowNumber));
            if (rowNumber == currentlyPlayingRow) {
                btn->setButtonText("stop");
                btn->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
                btn->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            } else {
                btn->setButtonText("play");
                btn->setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
                btn->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            }
        }
        else if (columnId == 6) {
            btn->setComponentID("delete_" + juce::String(rowNumber));
            btn->setButtonText("x");
            btn->setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            btn->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        }
    }
    else {
        delete existingComponentToUpdate;
        existingComponentToUpdate = nullptr;
    }
    return existingComponentToUpdate;
}

void PlaylistComponent::buttonClicked(juce::Button* button)
{
    juce::String idStr = button->getComponentID();
    if (idStr.startsWith("play_"))
    {
        int id = idStr.substring(5).getIntValue();
        if (id >= 0 && id < (int) playlistEntries.size() && player != nullptr)
        {
            if (currentlyPlayingRow == id)
            {
                                player->stop();
                currentlyPlayingRow = -1;
                tableComponent.deselectAllRows();
                tableComponent.updateContent();
            }
            else
            {
                juce::File trackFile{playlistEntries[id].getAudioFile()};

                if (trackFile.existsAsFile())
                {
                    if (onLoadToDeck) {
                        onLoadToDeck(trackFile);
                    }
                    currentlyPlayingRow = id;
                    if (onStartedPlayingLocally) onStartedPlayingLocally();
                    tableComponent.deselectAllRows();
                    tableComponent.updateContent();
                }
                else
                {
                    DBG("File does not exist: " << trackFile.getFullPathName());
                }
            }
            tableComponent.updateContent();
        }
    }
    else if (idStr.startsWith("delete_"))
    {
        int id = idStr.substring(7).getIntValue();
        if (id >= 0 && id < (int) playlistEntries.size())
        {
            if (currentlyPlayingRow == id && player != nullptr) {
                player->stop();
                currentlyPlayingRow = -1;
            } else if (currentlyPlayingRow > id) {
                currentlyPlayingRow--;
            }
            
            playlistEntries.erase(playlistEntries.begin() + id);
            tableComponent.updateContent();
        }
    }
}

juce::var PlaylistComponent::getDragSourceDescription (const juce::SparseSet<int> &currentlySelectedRows)
{
    if (currentlySelectedRows.size() > 0)
    {
        int selectedRow = currentlySelectedRows.getRange(0).getStart();
        if (selectedRow >= 0 && selectedRow < (int) playlistEntries.size())
        {
            return playlistEntries[selectedRow].getAudioFile();
        }
    }
    return {};
}

bool PlaylistComponent::isInterestedInFileDrag (const juce::StringArray &files)
{
    return true;
}

void PlaylistComponent::filesDropped (const juce::StringArray &files, int x, int y)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    for (const auto& f : files) {
        juce::File audioFile{f};
        if (audioFile.existsAsFile()) {
            PlaylistEntry entry{audioFile, formatManager};
            playlistEntries.push_back(entry);
        }
    }
    updateTable();
    savePlaylist();
}

juce::File getPlaylistDataFile()
{
    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                  .getChildFile("DJAudioPlayer");
    appDataFolder.createDirectory();
    return appDataFolder.getChildFile("playlist_data.txt");
}

void PlaylistComponent::savePlaylist()
{
    juce::File saveFile = getPlaylistDataFile();
    juce::String fileContent;

    for (const auto& entry : playlistEntries) {
        fileContent += entry.getAudioFile() + "\n";
    }

    saveFile.replaceWithText(fileContent);
}

void PlaylistComponent::loadPlaylist(juce::AudioFormatManager& formatManager)
{
    juce::File saveFile = getPlaylistDataFile();

    if (saveFile.existsAsFile())
    {
        juce::StringArray lines;
        saveFile.readLines(lines);

        for (const auto& line : lines)
        {
            juce::File trackFile{line.trim()};
            if (trackFile.existsAsFile())
            {
                PlaylistEntry entry{trackFile, formatManager};
                playlistEntries.push_back(entry);
            }
        }
        updateTable();
    }
}

void PlaylistComponent::setPlayingFile(const juce::String& filePath, bool isPlaying) {
    if (!isPlaying) {
        if (currentlyPlayingRow != -1 && currentlyPlayingRow < (int)playlistEntries.size()) {
            if (playlistEntries[currentlyPlayingRow].getAudioFile() == filePath) {
                currentlyPlayingRow = -1;
                tableComponent.updateContent();
            }
        }
        return;
    }
    
    if (currentlyPlayingRow >= 0 && currentlyPlayingRow < (int)playlistEntries.size()) {
        if (playlistEntries[currentlyPlayingRow].getAudioFile() == filePath) {
            tableComponent.deselectAllRows();
            tableComponent.updateContent();
            return;
        }
    }
    
    for (int i = 0; i < (int)playlistEntries.size(); ++i) {
        if (playlistEntries[i].getAudioFile() == filePath) {
            currentlyPlayingRow = i;
            tableComponent.deselectAllRows();
            tableComponent.updateContent();
            return;
        }
    }
    
    if (currentlyPlayingRow != -1) {
        currentlyPlayingRow = -1;
        tableComponent.updateContent();
    }
}
