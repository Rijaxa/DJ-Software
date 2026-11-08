/*
  ==============================================================================

    DeckGUI.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"

class ModernDialLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2.0f, height / 2.0f) - 6.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    
        g.setColour(juce::Colour(0xff252525));
        g.fillEllipse(rx, ry, rw, rw);
        
        g.setColour(juce::Colour(0xff121212));
        g.drawEllipse(rx, ry, rw, rw, 2.0f);
        
        juce::Path activeArc;
        activeArc.addCentredArc(centreX, centreY, radius - 4.0f, radius - 4.0f, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(juce::Colour(0xff00d2ff)); // Neon Cyan
        g.strokePath(activeArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        juce::Path p;
        auto pointerLength = radius - 4.0f;
        p.addRectangle(-2.0f, -pointerLength, 4.0f, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colours::white);
        g.fillPath(p);
    }
    
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto cornerSize = 4.0f;
        
        juce::Colour baseColour = backgroundColour;
        if (shouldDrawButtonAsDown || button.getToggleState())
            baseColour = baseColour.brighter(0.2f);
        else if (shouldDrawButtonAsHighlighted)
            baseColour = baseColour.brighter(0.1f);
            
        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, cornerSize);
        
        float alpha = (shouldDrawButtonAsDown || button.getToggleState() || shouldDrawButtonAsHighlighted) ? 1.0f : 0.4f;
        g.setColour(juce::Colour(0xff00d2ff).withAlpha(alpha));
        g.drawRoundedRectangle(bounds.reduced(1.0f), cornerSize, 2.0f);
    }
    
    void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::Font font (14.0f, juce::Font::bold);
        g.setFont (font);
        g.setColour (juce::Colours::white);
        
        auto bounds = button.getLocalBounds().reduced(2);
        g.drawFittedText (button.getButtonText(), bounds, juce::Justification::centred, 2);
    }
};

class HotCueButton : public juce::TextButton {
public:
    HotCueButton(int _index) : index(_index) {}
    
    std::function<void(int index, const juce::ModifierKeys& mods, bool isRightClick)> onClickWithMods;
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isPopupMenu()) {
            if (onClickWithMods)
                onClickWithMods(index, e.mods, true);
            return;
        }
        juce::TextButton::mouseDown(e);
    }
    
    void clicked(const juce::ModifierKeys& modifiers) override {
        juce::TextButton::clicked(modifiers);
        if (onClickWithMods)
            onClickWithMods(index, modifiers, false);
    }
    
    int getIndex() const { return index; }
    
private:
    int index;
};

class HotCueSlotComponent : public juce::Component {
public:
    HotCueSlotComponent(int _index) : mainButton(_index), index(_index) {
        addAndMakeVisible(mainButton);
        addChildComponent(clearButton);
        
        clearButton.setButtonText("x");
        clearButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xd0000000));
        clearButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffe74c3c));
        clearButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        
        clearButton.onClick = [this]() {
            if (onClearClicked)
                onClearClicked(index);
        };
    }
    
    void resized() override {
        mainButton.setBounds(getLocalBounds());
        int btnSize = juce::jlimit(16, 22, juce::jmin(getWidth() / 4, getHeight() / 2));
        clearButton.setBounds(getWidth() - btnSize - 2, 2, btnSize, btnSize);
    }
    
    HotCueButton mainButton;
    juce::TextButton clearButton;
    std::function<void(int index)> onClearClicked;
    int index;
};

class DeckGUI  : public juce::Component,
                 public juce::Button::Listener,
                 public juce::Slider::Listener,
                 public juce::FileDragAndDropTarget,
                 public juce::Timer,
                 public juce::DragAndDropTarget
{
public:
    DeckGUI(DJAudioPlayer* player,
            juce::AudioFormatManager & formatManagerToUse,
            juce::AudioThumbnailCache & cacheToUse,
            int deckId = 1);
    
    ~DeckGUI() override;

    void paint (juce::Graphics&) override;
    
    void resized() override;

    void buttonClicked (juce::Button* button) override;

    void sliderValueChanged (juce::Slider* slider) override;
    
    bool isInterestedInFileDrag (const juce::StringArray &files) override;
    
    void filesDropped (const juce::StringArray &files, int x, int y) override;
    
    void timerCallback() override;
    
    bool isInterestedInDragSource (const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    
    void itemDropped (const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    void loadTrack(const juce::File& file);
    
    void loadHotCuesForFile(const juce::String& filePath);
    
    std::function<void(const juce::String& filePath, bool isPlaying)> onPlayStateChanged;
    
    void saveHotCuesForFile(const juce::String& filePath);
    
    void updateHotCueButtonsUI();
    
    void handleHotCueClick(int index, const juce::ModifierKeys& mods, bool isRightClick);
    
    juce::File getHotCuesFile() const;

    void saveDeckState();
    
    void loadDeckState();

private:
    int deckId{1};
    
    juce::TextButton playButton{"PLAY"};
    juce::TextButton loadButton{"LOAD"};
    juce::TextButton loopButton{"LOOP"};
    juce::Slider volSlider;
    juce::Label volLabel{"volLabel", "Volume"};
    
    juce::Slider speedSlider;
    juce::Label speedLabel{"speedLabel", "Speed"};
    
    juce::Label trackNameLabel{"trackNameLabel", "No Track Loaded"};
    juce::Label timeLabel{"timeLabel", "00:00 / 00:00"};
    
    juce::OwnedArray<HotCueSlotComponent> hotCueSlots;
    juce::TextButton clearCuesButton{"CLEAR ALL CUES"};
    juce::String currentFilePath;

    std::unique_ptr<juce::FileChooser> fchooser;
    
    juce::TextButton tapBpmButton{"TAP BPM"};
    juce::Label bpmLabel{"bpmLabel", "BPM: ---"};
    std::vector<juce::uint32> tapTimes;
    void calculateBPM();

    
    DJAudioPlayer* player;
    
    WaveformDisplay waveformDisplay;
    ModernDialLookAndFeel modernDialLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckGUI)
};

