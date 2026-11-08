/*
  ==============================================================================

    WaveformDisplay.cpp

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay(juce::AudioFormatManager & formatManagerToUse,
                                 juce::AudioThumbnailCache & cacheToUse)  : audioThumb(1000, formatManagerToUse, cacheToUse),
                                                                            fileLoaded(false),
                                                                            position(0)
{
    audioThumb.addChangeListener(this);
}

WaveformDisplay::~WaveformDisplay() = default;
void WaveformDisplay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff181818));

    g.setColour (juce::Colour(0xff121212));
    g.drawRect (getLocalBounds(), 1);

    if (fileLoaded) {

        juce::ColourGradient gradient(juce::Colour(0xff005c8a), 0.0f, 0.0f, 
                                      juce::Colour(0xff005c8a), 0.0f, (float)getHeight(), false);
        gradient.addColour(0.5f, juce::Colour(0xff00d2ff));
        g.setGradientFill(gradient);
        
        audioThumb.drawChannel(g, getLocalBounds().reduced(2), 0, audioThumb.getTotalLength(), 0, 1.0f);
        
        if (!std::isnan(position) && !std::isinf(position)) {
            int playheadX = (int)(position * getWidth());
            
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.fillRect(0, 0, playheadX, getHeight());
            
            g.setColour(juce::Colour(0xff00d2ff).withAlpha(0.5f));
            g.fillRect(playheadX - 2, 0, 5, getHeight());
            
            g.setColour(juce::Colours::white);
            g.fillRect(playheadX, 0, 1, getHeight());
        }
    }
    else {
        g.setColour(juce::Colours::grey);
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.drawText ("NO TRACK LOADED", getLocalBounds(),
                    juce::Justification::centred, true);
    }
}

void WaveformDisplay::loadURL(juce::URL audioURL) {
    std::cout << "wfd: loadURL" << std::endl;
    
    audioThumb.clear();
    
    fileLoaded = audioThumb.setSource(new juce::URLInputSource(audioURL));
    
    if (fileLoaded) {
        std::cout << "wfd: loaded!" << std::endl;
        repaint();
    }
    else {
        std::cout << "wfd: not loaded!" << std::endl;
    }
}

void WaveformDisplay::changeListenerCallback (juce::ChangeBroadcaster *source) {
    std::cout << "wfd: change received!" << std::endl;
    
    repaint();
}

void WaveformDisplay::setPositionRelative(double pos)
{
    if (pos != position)
    {
        position = pos;
        repaint();
    }
}

void WaveformDisplay::mouseDown(const juce::MouseEvent& event)
{
    if (!fileLoaded) return;
    double newPos = (double)event.x / (double)getWidth();
    if (newPos < 0.0) newPos = 0.0;
    if (newPos > 1.0) newPos = 1.0;
    if (onPositionChanged)
        onPositionChanged(newPos);
}

void WaveformDisplay::mouseDrag(const juce::MouseEvent& event)
{
    if (!fileLoaded) return;
    double newPos = (double)event.x / (double)getWidth();
    if (newPos < 0.0) newPos = 0.0;
    if (newPos > 1.0) newPos = 1.0;
    if (onPositionChanged)
        onPositionChanged(newPos);
}
