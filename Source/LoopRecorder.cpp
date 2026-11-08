/*
  ==============================================================================

    LoopRecorder.cpp

  ==============================================================================
*/

#include "LoopRecorder.h"

LoopRecorder::LoopRecorder() {
    backgroundThread.startThread();
}

LoopRecorder::~LoopRecorder() {
    stopRecording();
    backgroundThread.stopThread(1000);
}

void LoopRecorder::startRecording(const juce::File& file, double sampleRate, int numChannels) {
    stopRecording();
    
    file.deleteFile();
    auto fileStream = file.createOutputStream();
    if (fileStream != nullptr) {
        juce::WavAudioFormat wavFormat;
        auto* writer = wavFormat.createWriterFor(fileStream.release(),
                                                 sampleRate,
                                                 (unsigned int) numChannels,
                                                 16,
                                                 {},
                                                 0);
        if (writer != nullptr) {
            threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768));
            isRecording = true;
            currentFile = file;
        }
    }
}

void LoopRecorder::stopRecording() {
    if (isRecording) {
        isRecording = false;
        threadedWriter.reset();
    }
}

void LoopRecorder::processBlock(const float* const* channelData, int numChannels, int numSamples) {
    if (isRecording.load() && threadedWriter != nullptr) {
        threadedWriter->write(channelData, numSamples);
    }
}

void LoopRecorder::processBuffer(const juce::AudioBuffer<float>& buffer, int numSamples) {
    if (isRecording.load() && threadedWriter != nullptr) {
        threadedWriter->write(buffer.getArrayOfReadPointers(), numSamples);
    }
}
