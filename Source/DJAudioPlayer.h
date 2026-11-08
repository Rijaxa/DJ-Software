/*
  ==============================================================================

    DJAudioPlayer.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>


class CustomTransport : public juce::PositionableAudioSource {
public:
    CustomTransport() = default;
    void setSource(juce::PositionableAudioSource* newSource) { source = newSource; }
    void setNextReadPosition(juce::int64 newPosition) override { if (source) source->setNextReadPosition(newPosition); }
    juce::int64 getNextReadPosition() const override { return source ? source->getNextReadPosition() : 0; }
    juce::int64 getTotalLength() const override { return source ? source->getTotalLength() : 0; }
    bool isLooping() const override { return source ? source->isLooping() : false; }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override { if (source) source->prepareToPlay(samplesPerBlockExpected, sampleRate); }
    void releaseResources() override { if (source) source->releaseResources(); }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        if (source && playing) {
            source->getNextAudioBlock(bufferToFill);
            bufferToFill.buffer->applyGain(bufferToFill.startSample, bufferToFill.numSamples, gain);
        } else {
            bufferToFill.clearActiveBufferRegion();
        }
    }
    void setGain(float newGain) { gain = newGain; }
    void setPlaying(bool shouldPlay) { playing = shouldPlay; }
    bool isPlaying() const { return playing; }
private:
    juce::PositionableAudioSource* source = nullptr;
    float gain = 1.0f;
    bool playing = false;
};

class DJAudioPlayer : public juce::AudioSource{

    public:
    DJAudioPlayer(juce::AudioFormatManager& formatManagerToUse);
    ~DJAudioPlayer() override;
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    
    void releaseResources() override;

    void loadURL(juce::URL audioURL);
    
    void setGain(double gain);

    void setSpeed(double ratio);
    
    void setPosition(double posInSecs);
    
    void setPositionRelative(double pos);
    
    void setLooping(bool shouldLoop);
    
    void start();
    
    void stop();

    bool isPlaying() const;

    double getPositionRelative();
    
    double getLengthInSeconds() const;
    
    double getCurrentPosition() const;
    double getHotCue(int index) const;
    void setHotCue(int index, double posInSecs);
    void clearHotCue(int index);
    void clearAllHotCues();
    void jumpToHotCue(int index);
    const std::array<double, 8>& getHotCues() const;
    void setHotCues(const std::array<double, 8>& cues);
    
    private:
    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    CustomTransport transportSource;
    juce::ResamplingAudioSource resampleSource{&transportSource, false, 2};
    std::array<double, 8> hotCues{-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
    bool isLoop = false;
    bool userPlaying = false;
};



