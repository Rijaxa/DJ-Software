/*
  ==============================================================================

    DeckGUI.cpp

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DeckGUI.h"

DeckGUI::DeckGUI(DJAudioPlayer* _player,
                 juce::AudioFormatManager & formatManageToUse,
                 juce::AudioThumbnailCache & cacheToUse,
                 int _deckId)
                 : player (_player),
                   waveformDisplay(formatManageToUse,cacheToUse),
                   deckId(_deckId)
{
    addAndMakeVisible(playButton);
    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(clearCuesButton);
    addAndMakeVisible(tapBpmButton);
    addAndMakeVisible(bpmLabel);
    
    playButton.addListener(this);
    loadButton.addListener(this);
    loopButton.addListener(this);
    loopButton.setClickingTogglesState(true);
    clearCuesButton.addListener(this);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff27ae60)); // Dark green
    loopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff333333));
    loopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    tapBpmButton.addListener(this);
    
    bpmLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    bpmLabel.setJustificationType(juce::Justification::centred);
    
    clearCuesButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffc0392b)); // Dark red
    clearCuesButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffe74c3c));
    
    addAndMakeVisible(volLabel);
    volLabel.setJustificationType(juce::Justification::centred);
    volLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    
    addAndMakeVisible(speedLabel);
    speedLabel.setJustificationType(juce::Justification::centred);
    speedLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    
    addAndMakeVisible(timeLabel);
    timeLabel.setJustificationType(juce::Justification::centred);
    timeLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    timeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d2ff)); // Neon cyan
    
    addAndMakeVisible(trackNameLabel);
    trackNameLabel.setJustificationType(juce::Justification::centred);
    trackNameLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    trackNameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    
    volSlider.addListener(this);
    speedSlider.addListener(this);
    
    volSlider.setRange(0.0, 5.0);
    volSlider.setValue(1.0);
    volSlider.setSliderStyle(juce::Slider::Rotary);
    
    playButton.setLookAndFeel(&modernDialLookAndFeel);
    loopButton.setLookAndFeel(&modernDialLookAndFeel);
    loadButton.setLookAndFeel(&modernDialLookAndFeel);
    clearCuesButton.setLookAndFeel(&modernDialLookAndFeel);
    tapBpmButton.setLookAndFeel(&modernDialLookAndFeel);
    
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
    loopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
    loadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
    tapBpmButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
    volSlider.setLookAndFeel(&modernDialLookAndFeel);
    volSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);

    speedSlider.setRange(0.1, 5.0, 0.01);
    speedSlider.setValue(1.0);
    speedSlider.setSliderStyle(juce::Slider::Rotary);
    speedSlider.setLookAndFeel(&modernDialLookAndFeel);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);

    waveformDisplay.onPositionChanged = [this](double pos) {
        if (player != nullptr) {
            player->setPositionRelative(pos);
        }
    };

    for (int i = 0; i < 8; ++i) {
        auto* slot = new HotCueSlotComponent(i);
        slot->mainButton.setButtonText("");
        slot->mainButton.onClickWithMods = [this](int index, const juce::ModifierKeys& mods, bool isRightClick) {
            handleHotCueClick(index, mods, isRightClick);
        };
        slot->onClearClicked = [this](int index) {
            if (player != nullptr) {
                player->clearHotCue(index);
                if (currentFilePath.isNotEmpty())
                    saveHotCuesForFile(currentFilePath);
                updateHotCueButtonsUI();
            }
        };
        hotCueSlots.add(slot);
        addAndMakeVisible(slot);
    }
    startTimer(40);
    updateHotCueButtonsUI();
}

DeckGUI::~DeckGUI()
{
    playButton.setLookAndFeel(nullptr);
    loopButton.setLookAndFeel(nullptr);
    loadButton.setLookAndFeel(nullptr);
    clearCuesButton.setLookAndFeel(nullptr);
    tapBpmButton.setLookAndFeel(nullptr);
    volSlider.setLookAndFeel(nullptr);
    speedSlider.setLookAndFeel(nullptr);
    stopTimer();
    saveDeckState();
}

void DeckGUI::paint (juce::Graphics& g)
{
    // Dark mode charcoal background
    g.fillAll (juce::Colour(0xff181818));   

    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds(), 1);
}

void DeckGUI::resized()
{
    double rowH = getHeight() / 16.0;
    double w = getWidth();
    int margin = 5;

    double btnH = rowH * 1.5;
    double topBtnW = (w - 4 * margin) / 3.0;
    
    playButton.setBounds(margin, margin, topBtnW, btnH);
    loopButton.setBounds(margin * 2 + topBtnW, margin, topBtnW, btnH);
    loadButton.setBounds(margin * 3 + topBtnW * 2, margin, topBtnW, btnH);

    double dialStartY = margin + btnH + margin;
    double dialAreaH = rowH * 4.0;
    
    volSlider.setBounds(0, dialStartY, w / 2.0, dialAreaH);
    speedSlider.setBounds(w / 2.0, dialStartY, w / 2.0, dialAreaH);
    
    volLabel.setBounds(0, dialStartY + dialAreaH, w / 2.0, rowH);
    speedLabel.setBounds(w / 2.0, dialStartY + dialAreaH, w / 2.0, rowH);

    trackNameLabel.setBounds(0, rowH * 6.7, w, rowH);
    timeLabel.setBounds(0, rowH * 7.5, w, rowH);
    waveformDisplay.setBounds(0, rowH * 8.5, w, rowH * 2.5);

    double cueH = rowH * 1.3;
    double cueW = w / 4.0;
    for (int i = 0; i < 4; ++i) {
        if (i < hotCueSlots.size())
            hotCueSlots[i]->setBounds(i * cueW + margin, rowH * 11.3, cueW - margin*2, cueH);
    }

    for (int i = 4; i < 8; ++i) {
        if (i < hotCueSlots.size())
            hotCueSlots[i]->setBounds((i - 4) * cueW + margin, rowH * 12.7, cueW - margin*2, cueH);
    }

    int bottomY = getHeight() - btnH - margin;
    clearCuesButton.setBounds(margin, bottomY, topBtnW, btnH);
    tapBpmButton.setBounds(margin * 2 + topBtnW, bottomY, topBtnW, btnH);
    bpmLabel.setBounds(margin * 3 + topBtnW * 2, bottomY, topBtnW, btnH);
}

void DeckGUI::loadTrack(const juce::File& file) {
    if (file.existsAsFile() && player != nullptr) {
        currentFilePath = file.getFullPathName();
        player->loadURL(juce::URL{file});
        waveformDisplay.loadURL(juce::URL{file});
        trackNameLabel.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
        loadHotCuesForFile(currentFilePath);
        saveDeckState();
    }
}

void DeckGUI::buttonClicked (juce::Button* button)
{
    if (player == nullptr)
        return;

    if (button == &playButton) {
        if (player->isPlaying()) {
            player->stop();
            playButton.setButtonText("PLAY");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff27ae60)); // Dark green
            if (onPlayStateChanged) onPlayStateChanged(currentFilePath, false);
        } else {
            player->start();
            playButton.setButtonText("STOP");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            if (onPlayStateChanged) onPlayStateChanged(currentFilePath, true);
        }
    }
    else if (button == &loopButton) {
        player->setLooping(loopButton.getToggleState());
    }
    else if (button == &loadButton) {
        fchooser = std::make_unique<juce::FileChooser> ("Select an audio file to play...",
                                                        juce::File{},
                                                        "*.wav;*.flac;*.mp3;*.aiff;*.m4a");
        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        fchooser->launchAsync (folderChooserFlags, [this] (const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file.exists()) {
                loadTrack(file);
            }
        });
    }
    else if (button == &clearCuesButton) {
        player->clearAllHotCues();
        updateHotCueButtonsUI();
        saveHotCuesForFile(currentFilePath);
    }
    else if (button == &tapBpmButton) {
        calculateBPM();
    }
}

void DeckGUI::sliderValueChanged (juce::Slider *slider) {
    if (player == nullptr)
        return;

    if (slider == &volSlider) {
        player->setGain(slider->getValue());
    }
    else if (slider == &speedSlider) {
        player->setSpeed(slider->getValue());
    }
}

bool DeckGUI::isInterestedInFileDrag (const juce::StringArray &files) {
    std::cout << "DeckGUI::isInterestedInFileDrag" << std::endl;
    return true;
}

void DeckGUI::filesDropped (const juce::StringArray &files, int x, int y){
    std::cout << "DeckGUI::filesDropped" << std::endl;
    if (files.size() > 0) {
        juce::File audioFile{files[0]};
        loadTrack(audioFile);
    }
}

void DeckGUI::timerCallback() {
    if (player != nullptr) {
        double relativePos = player->getPositionRelative();
        waveformDisplay.setPositionRelative(relativePos);
        
        double currentSecs = player->getCurrentPosition();
        double totalSecs = player->getLengthInSeconds();
        
        auto formatTime = [](double s) -> juce::String {
            if (s < 0.0) s = 0.0;
            int mins = (int)(s / 60.0);
            int secs = (int)s % 60;
            return juce::String::formatted("%02d:%02d", mins, secs);
        };
        
        timeLabel.setText(formatTime(currentSecs) + " / " + formatTime(totalSecs), juce::dontSendNotification);
        
        if (player->isPlaying() && playButton.getButtonText() == "PLAY") {
            playButton.setButtonText("STOP");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        } else if (!player->isPlaying() && playButton.getButtonText() == "STOP") {
            playButton.setButtonText("PLAY");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff27ae60));
            if (onPlayStateChanged) onPlayStateChanged(currentFilePath, false);
        }
    }
}

bool DeckGUI::isInterestedInDragSource (const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    return dragSourceDetails.description.toString().isNotEmpty();
}

void DeckGUI::itemDropped (const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    juce::String filePath = dragSourceDetails.description.toString();
    juce::File audioFile{filePath};
    loadTrack(audioFile);
}

juce::File DeckGUI::getHotCuesFile() const {
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    return appDataDir.getChildFile("AudioProj_hotcues.txt");
}

void DeckGUI::loadHotCuesForFile(const juce::String& filePath) {
    if (player == nullptr) return;
    
    std::array<double, 8> loadedCues;
    loadedCues.fill(-1.0);
    
    if (filePath.isNotEmpty()) {
        juce::File file = getHotCuesFile();
        if (file.existsAsFile()) {
            juce::StringArray lines;
            file.readLines(lines);
            
            for (const auto& line : lines) {
                if (line.startsWith(filePath + "|")) {
                    juce::String dataStr = line.substring((filePath + "|").length());
                    juce::StringArray tokens;
                    tokens.addTokens(dataStr, ",", "");
                    for (int i = 0; i < 8 && i < tokens.size(); ++i) {
                        loadedCues[i] = tokens[i].getDoubleValue();
                    }
                    break;
                }
            }
        }
    }
    
    player->setHotCues(loadedCues);
    updateHotCueButtonsUI();
}

void DeckGUI::saveHotCuesForFile(const juce::String& filePath) {
    if (filePath.isEmpty() || player == nullptr) return;
    
    juce::File file = getHotCuesFile();
    juce::StringArray lines;
    if (file.existsAsFile()) {
        file.readLines(lines);
    }
    
    const auto& cues = player->getHotCues();
    juce::String lineData = filePath + "|";
    for (int i = 0; i < 8; ++i) {
        lineData += juce::String(cues[i], 3);
        if (i < 7) lineData += ",";
    }
    
    bool found = false;
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].startsWith(filePath + "|")) {
            lines.set(i, lineData);
            found = true;
            break;
        }
    }
    
    if (!found) {
        lines.add(lineData);
    }
    
    file.replaceWithText(lines.joinIntoString("\n"));
}

static juce::String formatTimeSecs(double seconds) {
    if (seconds < 0.0) return "";
    int totalSecs = (int)seconds;
    int mins = totalSecs / 60;
    int secs = totalSecs % 60;
    int tenths = (int)((seconds - totalSecs) * 10.0);
    return juce::String::formatted("%02d:%02d.%d", mins, secs, tenths);
}

static const juce::Colour hotCueColors[8] = {
    juce::Colour(0xffe74c3c), // 1: Red
    juce::Colour(0xffe67e22), // 2: Orange
    juce::Colour(0xfff1c40f), // 3: Yellow
    juce::Colour(0xff2ecc71), // 4: Green
    juce::Colour(0xff1abc9c), // 5: Cyan
    juce::Colour(0xff3498db), // 6: Blue
    juce::Colour(0xff9b59b6), // 7: Purple
    juce::Colour(0xffe84393)  // 8: Pink
};

void DeckGUI::updateHotCueButtonsUI() {
    if (player == nullptr) return;
    
    const auto& cues = player->getHotCues();
    for (int i = 0; i < 8; ++i) {
        if (i >= hotCueSlots.size()) break;
        
        auto* slot = hotCueSlots[i];
        auto* btn = &(slot->mainButton);
        double t = cues[i];
        juce::Colour sigColor = hotCueColors[i];
        
        if (t >= 0.0) {
            btn->setButtonText(formatTimeSecs(t));
            btn->setColour(juce::TextButton::buttonColourId, sigColor);
            btn->setColour(juce::TextButton::buttonOnColourId, sigColor.darker(0.25f));
            if (i == 2) {
                btn->setColour(juce::TextButton::textColourOffId, juce::Colours::black);
            } else {
                btn->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            }
            slot->clearButton.setVisible(true);
        }
        else {
            btn->setButtonText("");
            btn->setColour(juce::TextButton::buttonColourId, sigColor.withAlpha(0.25f));
            btn->setColour(juce::TextButton::buttonOnColourId, sigColor.withAlpha(0.45f));
            slot->clearButton.setVisible(false);
        }
    }
}

void DeckGUI::handleHotCueClick(int index, const juce::ModifierKeys& mods, bool isRightClick) {
    if (player == nullptr) return;
    
    if (isRightClick) {
        juce::PopupMenu menu;
        menu.addItem(1, "Set / Overwrite CUE " + juce::String(index + 1));
        menu.addItem(2, "Clear CUE " + juce::String(index + 1));
        
        menu.showMenuAsync(juce::PopupMenu::Options{}, [this, index](int result) {
            if (result == 1) {
                double currentPos = player->getCurrentPosition();
                player->setHotCue(index, currentPos);
                if (currentFilePath.isNotEmpty())
                    saveHotCuesForFile(currentFilePath);
                updateHotCueButtonsUI();
            }
            else if (result == 2) {
                player->clearHotCue(index);
                if (currentFilePath.isNotEmpty())
                    saveHotCuesForFile(currentFilePath);
                updateHotCueButtonsUI();
            }
        });
        return;
    }

    double existingCue = player->getHotCue(index);
    if (mods.isShiftDown()) {
        double currentPos = player->getCurrentPosition();
        player->setHotCue(index, currentPos);
        if (currentFilePath.isNotEmpty())
            saveHotCuesForFile(currentFilePath);
        updateHotCueButtonsUI();
    }
    else {
        if (existingCue < 0.0) {
            double currentPos = player->getCurrentPosition();
            player->setHotCue(index, currentPos);
            if (currentFilePath.isNotEmpty())
                saveHotCuesForFile(currentFilePath);
            updateHotCueButtonsUI();
        }
        else {
            player->jumpToHotCue(index);
            timerCallback();
        }
    }
}

static juce::File getDeckStateFile(int id) {
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    return appDataDir.getChildFile("AudioProj_deck_" + juce::String(id) + "_state.txt");
}

void DeckGUI::saveDeckState() {
    if (currentFilePath.isNotEmpty()) {
        juce::File file = getDeckStateFile(deckId);
        file.replaceWithText(currentFilePath);
    }
}

void DeckGUI::loadDeckState() {
    juce::File file = getDeckStateFile(deckId);
    if (file.existsAsFile()) {
        juce::String savedPath = file.loadFileAsString().trim();
        if (savedPath.isNotEmpty()) {
            juce::File audioFile{savedPath};
            if (audioFile.existsAsFile()) {
                loadTrack(audioFile);
            }
        }
    }
}

void DeckGUI::calculateBPM()
{
    juce::uint32 now = juce::Time::getMillisecondCounter();
    
    if (!tapTimes.empty() && (now - tapTimes.back() > 2000)) {
        tapTimes.clear();
    }
    
    tapTimes.push_back(now);
    
    if (tapTimes.size() >= 4) {
        if (tapTimes.size() > 8) {
            tapTimes.erase(tapTimes.begin());
        }
        
        double totalDuration = tapTimes.back() - tapTimes.front();
        int intervals = (int)tapTimes.size() - 1;
        double averageIntervalMs = totalDuration / intervals;
        
        if (averageIntervalMs > 0) {
            double bpm = 60000.0 / averageIntervalMs;
            bpmLabel.setText("BPM: " + juce::String(bpm, 1), juce::dontSendNotification);
        }
    } else if (tapTimes.size() == 1) {
        bpmLabel.setText("BPM: Tap...", juce::dontSendNotification);
    }
}
