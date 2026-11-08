/*
  ==============================================================================

    DJAudioPlayer.cpp

  ==============================================================================
*/

#include "DJAudioPlayer.h"

DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& _formatManager): formatManager(_formatManager) {}
DJAudioPlayer::~DJAudioPlayer() = default;

void DJAudioPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
    //formatManager.registerBasicFormats();
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}
void DJAudioPlayer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill){
    resampleSource.getNextAudioBlock(bufferToFill);
}
void DJAudioPlayer::releaseResources() {
    transportSource.releaseResources();
    resampleSource.releaseResources();
}
void DJAudioPlayer::loadURL(juce::URL audioURL) {
    juce::AudioFormatReader* reader = nullptr;
    if (audioURL.isLocalFile())
    {
        reader = formatManager.createReaderFor(audioURL.getLocalFile());
    }
    else
    {
        reader = formatManager.createReaderFor(audioURL.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)));
    }

    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource (new juce::AudioFormatReaderSource(reader, true));
        newSource->setLooping(isLoop);
        transportSource.setSource (nullptr);
        transportSource.setSource (newSource.get());
        readerSource.reset(newSource.release());
    }
}
void DJAudioPlayer::setGain(double gain) {
    transportSource.setGain(gain);
}
void DJAudioPlayer::setSpeed(double ratio){
    if (ratio <= 0.0 || ratio > 10.0) {
        std::cout << "DJAudioPlayer::setSpeed ratio should be between 0 and 10" <<std::endl;
    }
    else {
        resampleSource.setResamplingRatio(ratio);
    }
}
void DJAudioPlayer::setPosition(double posInSecs) {
    if (readerSource) {
        transportSource.setNextReadPosition(posInSecs * readerSource->getAudioFormatReader()->sampleRate);
    }
}

void DJAudioPlayer::setPositionRelative(double pos) {
    if (pos < 0 || pos > 1.0) {
        std::cout << "DJAudioPlayer::setPositionRelative pos should be between 0 and 1" <<std::endl;
    }
    else {
        double posInSecs = getLengthInSeconds() * pos;
        setPosition(posInSecs);
    }
}

void DJAudioPlayer::start() {
    userPlaying = true;
    transportSource.setPlaying(true);
    if (transportSource.getTotalLength() > 0 && transportSource.getNextReadPosition() >= transportSource.getTotalLength()) {
        transportSource.setNextReadPosition(0);
    }
}
void DJAudioPlayer::stop() {
    userPlaying = false;
    transportSource.setPlaying(false);
}

double DJAudioPlayer::getPositionRelative() {
    double len = getLengthInSeconds();
    if (len > 0.0) return getCurrentPosition() / len;
    return 0.0;
}
double DJAudioPlayer::getLengthInSeconds() const {
    if (readerSource) return readerSource->getTotalLength() / readerSource->getAudioFormatReader()->sampleRate;
    return 0.0;
}

double DJAudioPlayer::getCurrentPosition() const {
    if (readerSource) return transportSource.getNextReadPosition() / readerSource->getAudioFormatReader()->sampleRate;
    return 0.0;
}
double DJAudioPlayer::getHotCue(int index) const {
    if (index >= 0 && index < 8) {
        return hotCues[index];
    }
    return -1.0;
}

void DJAudioPlayer::setHotCue(int index, double posInSecs) {
    if (index >= 0 && index < 8) {
        hotCues[index] = posInSecs;
    }
}

void DJAudioPlayer::clearHotCue(int index) {
    if (index >= 0 && index < 8) {
        hotCues[index] = -1.0;
    }
}

void DJAudioPlayer::clearAllHotCues() {
    hotCues.fill(-1.0);
}

void DJAudioPlayer::jumpToHotCue(int index) {
    if (index >= 0 && index < 8 && hotCues[index] >= 0.0) {
        setPosition(hotCues[index]);
    }
}

const std::array<double, 8>& DJAudioPlayer::getHotCues() const {
    return hotCues;
}

void DJAudioPlayer::setHotCues(const std::array<double, 8>& cues) {
    hotCues = cues;
}


bool DJAudioPlayer::isPlaying() const {
    if (userPlaying && !isLoop && transportSource.getTotalLength() > 0) {
        if (transportSource.getNextReadPosition() >= transportSource.getTotalLength()) {
            return false;
        }
    }
    return userPlaying;
}

void DJAudioPlayer::setLooping(bool shouldLoop) {
    if (readerSource != nullptr) {
        if (!shouldLoop) {
            juce::int64 len = readerSource->getTotalLength();
            if (len > 0) {
                readerSource->setNextReadPosition(readerSource->getNextReadPosition() % len);
            }
        }
        readerSource->setLooping(shouldLoop);
    }
    isLoop = shouldLoop;
}
