/*
  ==============================================================================

    MainComponent.cpp

  ==============================================================================
*/

#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize (1000, 650);

    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels (2, 2);
    }
    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(deckGUI2);
    
    addAndMakeVisible(musicLibrary);
    addAndMakeVisible(loopSamplerPanel);
    addAndMakeVisible(sessionManager);
    
    formatManager.registerBasicFormats();
    deckGUI1.loadDeckState();
    deckGUI2.loadDeckState();

    loopSamplerPanel.onLoadSampleToDeck = [this](int deckNum, const juce::File& file) {
        if (deckNum == 1) {
            deckGUI1.loadTrack(file);
        } else if (deckNum == 2) {
            deckGUI2.loadTrack(file);
        }
    };
    
    deckGUI1.onPlayStateChanged = [this](const juce::String& filePath, bool isPlaying) {
        musicLibrary.getPlaylistComponent()->setPlayingFile(filePath, isPlaying);
        loopSamplerPanel.setPlayingFile(filePath, isPlaying);
    };
    deckGUI2.onPlayStateChanged = [this](const juce::String& filePath, bool isPlaying) {
        musicLibrary.getPlaylistComponent()->setPlayingFile(filePath, isPlaying);
        loopSamplerPanel.setPlayingFile(filePath, isPlaying);
    };
    
        musicLibrary.getPlaylistComponent()->onLoadToDeck = [this](juce::File file) {
        deckGUI1.loadTrack(file);
        player1.start();
        deckGUI1.timerCallback();
    };

    musicLibrary.getPlaylistComponent()->onStartedPlayingLocally = [this]() {
        loopSamplerPanel.clearHighlight();
    };

    loopSamplerPanel.onStartedPlayingLocally = [this]() {
        musicLibrary.getPlaylistComponent()->clearHighlight();
    };
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    loopSamplerPanel.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (loopSamplerPanel.isRecording() && bufferToFill.buffer != nullptr && bufferToFill.numSamples > 0)
    {
        int numInputCh = bufferToFill.buffer->getNumChannels();
        if (numInputCh > 0)
        {
            loopSamplerPanel.processMicAudio(bufferToFill.buffer->getArrayOfReadPointers(), numInputCh, bufferToFill.numSamples);
        }
    }

    bufferToFill.clearActiveBufferRegion();

    juce::AudioBuffer<float> tempBuffer1(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
    tempBuffer1.clear();
    juce::AudioSourceChannelInfo tempInfo1(&tempBuffer1, 0, bufferToFill.numSamples);
    player1.getNextAudioBlock(tempInfo1);

    juce::AudioBuffer<float> tempBuffer2(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
    tempBuffer2.clear();
    juce::AudioSourceChannelInfo tempInfo2(&tempBuffer2, 0, bufferToFill.numSamples);
    player2.getNextAudioBlock(tempInfo2);

    if (loopSamplerPanel.isRecording())
    {
        int selectedSrc = loopSamplerPanel.getSelectedSourceIndex();
        if (selectedSrc == 1)
        {
            loopSamplerPanel.processDeck1Audio(tempBuffer1, bufferToFill.numSamples);
        }
        else if (selectedSrc == 2)
        {
            loopSamplerPanel.processDeck2Audio(tempBuffer2, bufferToFill.numSamples);
        }
    }

    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
    {
        bufferToFill.buffer->addFrom(ch, bufferToFill.startSample, tempBuffer1, ch, 0, bufferToFill.numSamples);
        bufferToFill.buffer->addFrom(ch, bufferToFill.startSample, tempBuffer2, ch, 0, bufferToFill.numSamples);
    }

    juce::AudioBuffer<float> previewBuffer(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
    previewBuffer.clear();
    juce::AudioSourceChannelInfo previewInfo(&previewBuffer, 0, bufferToFill.numSamples);
    loopSamplerPanel.getNextAudioBlock(previewInfo);

    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
    {
        bufferToFill.buffer->addFrom(ch, bufferToFill.startSample, previewBuffer, ch, 0, bufferToFill.numSamples);
    }

    if (loopSamplerPanel.isRecording() && loopSamplerPanel.getSelectedSourceIndex() == 3)
    {
        loopSamplerPanel.processMasterAudio(*bufferToFill.buffer, bufferToFill.numSamples);
    }
    
    masterRecorder.processBuffer(*bufferToFill.buffer, bufferToFill.numSamples);
}

void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();
    mixerSource.releaseResources();
    loopSamplerPanel.releaseResources();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void MainComponent::resized()
{
    int margin = 6;
    int w = getWidth();
    int h = getHeight();
    int sessionH = 50;
    
    sessionManager.setBounds(margin, h - sessionH - margin, w - margin * 2, sessionH);
    
    int remainingH = h - sessionH - margin * 2;
    int topH = (int)(remainingH * 0.60);
    int bottomH = remainingH - topH;
    int leftW = w / 2;
    int rightW = w - leftW;

    deckGUI1.setBounds(margin, margin, leftW - margin * 1.5, topH - margin * 1.5);
    deckGUI2.setBounds(leftW + margin * 0.5, margin, rightW - margin * 1.5, topH - margin * 1.5);
    
    musicLibrary.setBounds(margin, topH + margin * 0.5, leftW - margin * 1.5, bottomH - margin * 1.5);
    loopSamplerPanel.setBounds(leftW + margin * 0.5, topH + margin * 0.5, rightW - margin * 1.5, bottomH - margin * 1.5);
}

