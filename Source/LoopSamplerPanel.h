/*
  ==============================================================================

    LoopSamplerPanel.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SampleEntry.h"
#include "LoopRecorder.h"

class LoopSamplerPanel : public juce::Component,
                         public juce::TableListBoxModel,
                         public juce::Button::Listener,
                         public juce::AudioSource
{
public:
    std::function<void()> onStartedPlayingLocally;
    void clearHighlight() { currentlyPlayingRow = -1; juce::MessageManager::callAsync([this]() { tableComponent.updateContent(); }); }
    LoopSamplerPanel(juce::AudioFormatManager& formatManagerToUse);
    ~LoopSamplerPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

    void buttonClicked(juce::Button* button) override;
    
    juce::var getDragSourceDescription (const juce::SparseSet<int> &currentlySelectedRows) override;

    void startRecording(double sampleRate);
    void setPlayingFile(const juce::String& filePath, bool isPlaying);
    void stopRecording();
    bool isRecording() const;
    int getSelectedSourceIndex() const;

    std::function<void(int deckNumber, const juce::File& file)> onLoadSampleToDeck;

    void processMicAudio(const float* const* channelData, int numChannels, int numSamples);
    void processDeck1Audio(const juce::AudioBuffer<float>& buffer, int numSamples);
    void processDeck2Audio(const juce::AudioBuffer<float>& buffer, int numSamples);
    void processMasterAudio(const juce::AudioBuffer<float>& buffer, int numSamples);

    juce::File getSamplesDirectory() const;

private:
    juce::AudioFormatManager& formatManager;
    
    juce::Label titleLabel;
    juce::ComboBox sourceSelector;
    juce::TextButton recButton{juce::String(juce::CharPointer_UTF8("🔴 REC START"))};
    juce::Slider volSlider;
    juce::Label volLabel;
        juce::TableListBox tableComponent;

    std::vector<SampleEntry> sampleEntries;
    LoopRecorder recorder;

    juce::AudioTransportSource loopTransportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> loopReaderSource;
    int currentlyPlayingRow{-1};

    juce::File getManifestFile() const;
    void loadSamplesManifest();
    void saveSamplesManifest();
    void removeSample(int rowNumber);
    void playPreviewSample(int rowNumber);
    void stopPreview();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopSamplerPanel)
};
