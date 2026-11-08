/*
  ==============================================================================

    LoopSamplerPanel.cpp

  ==============================================================================
*/

#include "LoopSamplerPanel.h"

LoopSamplerPanel::LoopSamplerPanel(juce::AudioFormatManager& formatManagerToUse)
    : formatManager(formatManagerToUse)
{
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Recordings", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));

    addAndMakeVisible(sourceSelector);
    sourceSelector.addItem("Microphone", 1);
    sourceSelector.addItem("Deck 1", 2);
    sourceSelector.addItem("Deck 2", 3);
    sourceSelector.addItem("Master Mix", 4);
    sourceSelector.setSelectedId(1);

    addAndMakeVisible(recButton);
    recButton.addListener(this);
    recButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffc0392b));
    recButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffe74c3c));

    addAndMakeVisible(volSlider);
    volSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volSlider.setRange(0.0, 5.0, 0.01);
    volSlider.setValue(0.75);
    volSlider.onValueChange = [this]() {
        loopTransportSource.setGain((float)volSlider.getValue());
    };
    loopTransportSource.setGain(0.75f);
    
    addAndMakeVisible(volLabel);
    volLabel.setText("Vol:", juce::dontSendNotification);
    volLabel.attachToComponent(&volSlider, true);


    tableComponent.getHeader().addColumn("Sample Name", 1, 130);
    tableComponent.getHeader().addColumn("Source",      2, 75);
    tableComponent.getHeader().addColumn("Duration",    3, 65);
    tableComponent.getHeader().addColumn("Play",        4, 65);
    tableComponent.getHeader().addColumn("Volume",      5, 120);
    tableComponent.getHeader().addColumn("Delete",      6, 50);

    tableComponent.getHeader().setStretchToFitActive(true);
    tableComponent.setModel(this);
    addAndMakeVisible(tableComponent);

    loadSamplesManifest();
}

LoopSamplerPanel::~LoopSamplerPanel()
{
    stopPreview();
}

void LoopSamplerPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff181818));
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

void LoopSamplerPanel::resized()
{
    double headerH = 32.0;
    double w = getWidth();

    int btnW = 125;
    int comboW = 115;
    int volW = 80;
    
    titleLabel.setBounds(5, 2, 160, headerH - 4);
    recButton.setBounds(w - btnW - 5, 4, btnW, headerH - 8);
    
    volSlider.setBounds(recButton.getX() - volW - 5, 4, volW, headerH - 8);
    sourceSelector.setBounds(volSlider.getX() - 40 - comboW - 10, 4, comboW, headerH - 8);

    tableComponent.setBounds(0, headerH, w, getHeight() - headerH);
}

void LoopSamplerPanel::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    loopTransportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void LoopSamplerPanel::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    loopTransportSource.getNextAudioBlock(bufferToFill);
}

void LoopSamplerPanel::releaseResources()
{
    loopTransportSource.releaseResources();
}

int LoopSamplerPanel::getNumRows()
{
    return (int) sampleEntries.size();
}

void LoopSamplerPanel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected) {
        g.fillAll(juce::Colours::orange);
    } else {
        g.fillAll(juce::Colour(0xff2d2d2d));
    }
}

void LoopSamplerPanel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= (int) sampleEntries.size())
        return;

    const auto& entry = sampleEntries[rowNumber];
    if (rowNumber == currentlyPlayingRow || rowIsSelected)
        g.setColour(juce::Colours::black);
    else
        g.setColour(juce::Colours::white);
    g.setFont(12.0f);

    if (columnId == 1) {
        g.drawText(entry.getSampleName(), 4, 0, width - 8, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 2) {
        g.drawText(entry.getSourceName(), 2, 0, width - 4, height, juce::Justification::centred, true);
    }
    else if (columnId == 3) {
        double sec = entry.getDuration();
        int mins = (int)sec / 60;
        int secs = (int)sec % 60;
        juce::String durStr = juce::String::formatted("%02d:%02d", mins, secs);
        g.drawText(durStr, 2, 0, width - 4, height, juce::Justification::centred, true);
    }
}

juce::Component* LoopSamplerPanel::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate)
{
    if (columnId == 4 || columnId == 6) {
        if (existingComponentToUpdate == nullptr || dynamic_cast<juce::TextButton*>(existingComponentToUpdate) == nullptr) {
            delete existingComponentToUpdate;
            juce::TextButton* btn = new juce::TextButton();
            btn->addListener(this);
            existingComponentToUpdate = btn;
        }

        auto* btn = static_cast<juce::TextButton*>(existingComponentToUpdate);
        
        if (columnId == 4) {
            btn->setComponentID("preview_" + juce::String(rowNumber));
            if (rowNumber == currentlyPlayingRow) {
                btn->setButtonText(juce::String(juce::CharPointer_UTF8("⏹ stop")));
                btn->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            } else {
                btn->setButtonText(juce::String(juce::CharPointer_UTF8("▶ play")));
                btn->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff27ae60));
            }
        }
        else if (columnId == 6) {
            btn->setComponentID("remove_" + juce::String(rowNumber));
            btn->setButtonText("x");
            btn->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff7f8c8d));
        }
    }
    else if (columnId == 5) {
        if (existingComponentToUpdate == nullptr || dynamic_cast<juce::Slider*>(existingComponentToUpdate) == nullptr) {
            delete existingComponentToUpdate;
            juce::Slider* slider = new juce::Slider();
            slider->setSliderStyle(juce::Slider::LinearHorizontal);
            slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider->setRange(0.0, 3.0, 0.01);
            existingComponentToUpdate = slider;
        }
        
        auto* slider = static_cast<juce::Slider*>(existingComponentToUpdate);
        if (rowNumber >= 0 && rowNumber < sampleEntries.size()) {
            slider->setValue(sampleEntries[rowNumber].getVolume(), juce::dontSendNotification);
            slider->onValueChange = [this, rowNumber, slider]() {
                if (rowNumber >= 0 && rowNumber < sampleEntries.size()) {
                    sampleEntries[rowNumber].setVolume(slider->getValue());
                    if (currentlyPlayingRow == rowNumber) {
                        loopTransportSource.setGain((float)slider->getValue());
                    }
                    saveSamplesManifest();
                }
            };
        }
    }
    else {
        delete existingComponentToUpdate;
        existingComponentToUpdate = nullptr;
    }
    return existingComponentToUpdate;
}

juce::var LoopSamplerPanel::getDragSourceDescription (const juce::SparseSet<int> &currentlySelectedRows)
{
    if (currentlySelectedRows.size() > 0)
    {
        int selectedRow = currentlySelectedRows.getRange(0).getStart();
        if (selectedRow >= 0 && selectedRow < (int) sampleEntries.size())
        {
            return sampleEntries[selectedRow].getSampleFile().getFullPathName();
        }
    }
    return {};
}

void LoopSamplerPanel::buttonClicked(juce::Button* button)
{
    if (button == &recButton) {
        if (recorder.isCurrentlyRecording()) {
            stopRecording();
        } else {
            startRecording(44100.0);
        }
        return;
    }

    juce::String idStr = button->getComponentID();
    if (idStr.startsWith("preview_")) {
        int row = idStr.substring(8).getIntValue();
        if (currentlyPlayingRow == row) {
            stopPreview();
        } else {
            playPreviewSample(row);
        }
        tableComponent.updateContent();
    }
    else if (idStr.startsWith("remove_")) {
        int row = idStr.substring(7).getIntValue();
        removeSample(row);
    }
}

void LoopSamplerPanel::startRecording(double sampleRate)
{
    juce::File samplesDir = getSamplesDirectory();
    juce::Time now = juce::Time::getCurrentTime();
    juce::String timestampStr = juce::String::formatted("%04d%02d%02d_%02d%02d%02d",
                                                       now.getYear(),
                                                       now.getMonth() + 1,
                                                       now.getDayOfMonth(),
                                                       now.getHours(),
                                                       now.getMinutes(),
                                                       now.getSeconds());
    juce::String fileName = "Sample_" + timestampStr + ".wav";
    juce::File sampleFile = samplesDir.getChildFile(fileName);

    recorder.startRecording(sampleFile, sampleRate);

    recButton.setButtonText(juce::String(juce::CharPointer_UTF8("⏹ RECORDING")));
    recButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
}

void LoopSamplerPanel::stopRecording()
{
    recorder.stopRecording();
    recButton.setButtonText(juce::String(juce::CharPointer_UTF8("🔴 REC START")));
    recButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffc0392b));

    juce::File recFile = recorder.getCurrentFile();
    std::cout << "Recording stopped. File: " << recFile.getFullPathName() << " Size: " << recFile.getSize() << std::endl;
    if (recFile.existsAsFile() && recFile.getSize() > 44) {
        juce::String srcName = sourceSelector.getText();
        SampleEntry newEntry{recFile, srcName, formatManager};
        sampleEntries.push_back(newEntry);
        saveSamplesManifest();
        tableComponent.updateContent();
        std::cout << "Successfully added recording to table." << std::endl;
    } else {
        std::cout << "Failed to add recording! Exists: " << (int)recFile.existsAsFile() << std::endl;
    }
}

bool LoopSamplerPanel::isRecording() const
{
    return recorder.isCurrentlyRecording();
}

int LoopSamplerPanel::getSelectedSourceIndex() const
{
    return sourceSelector.getSelectedId() - 1;
}

void LoopSamplerPanel::processMicAudio(const float* const* channelData, int numChannels, int numSamples)
{
    if (getSelectedSourceIndex() == 0) {
        recorder.processBlock(channelData, numChannels, numSamples);
    }
}

void LoopSamplerPanel::processDeck1Audio(const juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (getSelectedSourceIndex() == 1) {
        recorder.processBuffer(buffer, numSamples);
    }
}

void LoopSamplerPanel::processDeck2Audio(const juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (getSelectedSourceIndex() == 2) {
        recorder.processBuffer(buffer, numSamples);
    }
}

void LoopSamplerPanel::processMasterAudio(const juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (getSelectedSourceIndex() == 3) {
        recorder.processBuffer(buffer, numSamples);
    }
}

juce::File LoopSamplerPanel::getSamplesDirectory() const
{
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("DJAudioPlayer");
    appDataDir.createDirectory();
    juce::File samplesDir = appDataDir.getChildFile("Samples");
    if (!samplesDir.exists())
        samplesDir.createDirectory();
    return samplesDir;
}

juce::File LoopSamplerPanel::getManifestFile() const
{
    return getSamplesDirectory().getChildFile("samples_manifest.txt");
}

void LoopSamplerPanel::loadSamplesManifest()
{
    sampleEntries.clear();
    juce::File manifestFile = getManifestFile();
    if (manifestFile.existsAsFile()) {
        juce::StringArray lines;
        manifestFile.readLines(lines);

        for (const auto& line : lines) {
            juce::StringArray tokens;
            tokens.addTokens(line, "|", "");
            if (tokens.size() >= 4) {
                juce::File audioFile{tokens[0]};
                if (audioFile.existsAsFile()) {
                    double vol = tokens.size() > 4 ? tokens[4].getDoubleValue() : 1.0;
                    SampleEntry entry{tokens[0], tokens[1], tokens[2], tokens[3].getDoubleValue(), vol};
                    sampleEntries.push_back(entry);
                }
            }
        }
    }
    tableComponent.updateContent();
}

void LoopSamplerPanel::saveSamplesManifest()
{
    juce::File manifestFile = getManifestFile();
    juce::StringArray lines;

    for (const auto& entry : sampleEntries) {
        juce::String line = entry.getSampleFile().getFullPathName() + "|"
                          + entry.getSampleName() + "|"
                          + entry.getSourceName() + "|"
                          + juce::String(entry.getDuration(), 3) + "|"
                          + juce::String(entry.getVolume(), 3);
        lines.add(line);
    }

    manifestFile.replaceWithText(lines.joinIntoString("\n"));
}

void LoopSamplerPanel::removeSample(int rowNumber)
{
    if (rowNumber >= 0 && rowNumber < (int) sampleEntries.size()) {
        if (currentlyPlayingRow == rowNumber) {
            stopPreview();
        }

        sampleEntries[rowNumber].getSampleFile().deleteFile();
        sampleEntries.erase(sampleEntries.begin() + rowNumber);

        if (currentlyPlayingRow > rowNumber) {
            currentlyPlayingRow--;
        }

        saveSamplesManifest();
        tableComponent.updateContent();
    }
}

void LoopSamplerPanel::playPreviewSample(int rowNumber)
{
    stopPreview();

    if (rowNumber >= 0 && rowNumber < (int) sampleEntries.size()) {
        juce::File audioFile = sampleEntries[rowNumber].getSampleFile();
        auto* reader = formatManager.createReaderFor(audioFile);
        if (reader != nullptr) {
            std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));
            newSource->setLooping(true); // Loop audio!
            loopTransportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            loopReaderSource.reset(newSource.release());
            loopTransportSource.setPosition(0);
            loopTransportSource.setGain((float)sampleEntries[rowNumber].getVolume());
            loopTransportSource.start();
            currentlyPlayingRow = rowNumber;
            tableComponent.deselectAllRows();
        }
    }
}

void LoopSamplerPanel::stopPreview()
{
    loopTransportSource.stop();
    loopTransportSource.setSource(nullptr);
    loopReaderSource.reset();
    currentlyPlayingRow = -1;
    juce::MessageManager::callAsync([this]() { tableComponent.deselectAllRows(); });
}

void LoopSamplerPanel::setPlayingFile(const juce::String& filePath, bool isPlaying) {
    if (!isPlaying) {
        if (currentlyPlayingRow != -1 && currentlyPlayingRow < (int)sampleEntries.size()) {
            if (sampleEntries[currentlyPlayingRow].getSampleFile().getFullPathName() == filePath) {
                currentlyPlayingRow = -1;
                juce::MessageManager::callAsync([this]() { tableComponent.updateContent(); });
            }
        }
        return;
    }
    
    if (currentlyPlayingRow >= 0 && currentlyPlayingRow < (int)sampleEntries.size()) {
        if (sampleEntries[currentlyPlayingRow].getSampleFile().getFullPathName() == filePath) {
            juce::MessageManager::callAsync([this]() {
                tableComponent.deselectAllRows();
                tableComponent.updateContent();
            });
            return;
        }
    }
    
    for (int i = 0; i < (int)sampleEntries.size(); ++i) {
        if (sampleEntries[i].getSampleFile().getFullPathName() == filePath) {
            currentlyPlayingRow = i;
            juce::MessageManager::callAsync([this]() {
                tableComponent.deselectAllRows();
                tableComponent.updateContent();
            });
            return;
        }
    }
}
