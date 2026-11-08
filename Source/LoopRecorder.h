/*
  ==============================================================================

    LoopRecorder.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class LoopRecorder {
public:
    LoopRecorder();
    
    ~LoopRecorder();
    
    void startRecording(const juce::File& file, double sampleRate, int numChannels = 2);
    
    void stopRecording();

    void processBlock(const float* const* channelData, int numChannels, int numSamples);
    
    void processBuffer(const juce::AudioBuffer<float>& buffer, int numSamples);
    
    bool isCurrentlyRecording() const { return isRecording.load(); }
    
    juce::File getCurrentFile() const { return currentFile; }

private:
    juce::TimeSliceThread backgroundThread{"AudioRecorderThread"};
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    std::atomic<bool> isRecording{false};
    juce::File currentFile;
};
