/*
  ==============================================================================

    WaveformDisplay.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class WaveformDisplay  : public juce::Component,
                         public juce::ChangeListener

{
public:
    WaveformDisplay(juce::AudioFormatManager & formatManagerToUse,
                    juce::AudioThumbnailCache & cacheToUse);
    ~WaveformDisplay() override;

    void paint (juce::Graphics&) override;
    
    void changeListenerCallback (juce::ChangeBroadcaster *source) override;
    
    void loadURL (juce::URL audioURL);
    
    void setPositionRelative(double pos);

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    std::function<void(double)> onPositionChanged;

private:
    juce::AudioThumbnail audioThumb;
    bool fileLoaded;
    double position;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay);
};
