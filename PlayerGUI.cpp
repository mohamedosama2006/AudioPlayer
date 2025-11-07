#include "PlayerGUI.h"


PlayerGUI::PlaylistComponent::PlaylistComponent()
{
    addAndMakeVisible(listBox);
    listBox.setModel(this);
    listBox.setRowHeight(24);

    addAndMakeVisible(addFilesButton);
    addFilesButton.setButtonText("Add Files");
    addFilesButton.addListener(this);
}

PlayerGUI::PlaylistComponent::~PlaylistComponent() {}

void PlayerGUI::PlaylistComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawFittedText("Playlist", getLocalBounds().removeFromTop(26).reduced(4), juce::Justification::centredLeft, 1);
}

void PlayerGUI::PlaylistComponent::resized()
{
    auto r = getLocalBounds().reduced(6);
    addFilesButton.setBounds(r.removeFromBottom(28));
    listBox.setBounds(r.removeFromTop(getHeight() - 40));
}

int PlayerGUI::PlaylistComponent::getNumRows()
{
    return playlist.size();
}

void PlayerGUI::PlaylistComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= playlist.size()) return;
    g.fillAll(rowIsSelected ? juce::Colours::lightblue : juce::Colours::transparentBlack);
    g.setColour(juce::Colours::white);
    g.drawText(playlist[rowNumber].getFileName(), 4, 0, width - 8, height, juce::Justification::centredLeft);
}

void PlayerGUI::PlaylistComponent::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < playlist.size())
    {
        listBox.selectRow(row);

        auto selectedFile = getSelectedFile();
        if (selectedFile.existsAsFile())
        {
            DBG("Selected file: " << selectedFile.getFullPathName());
        }
    }
}

void PlayerGUI::PlaylistComponent::addFiles(const juce::Array<juce::File>& files)
{
    for (const auto& f : files)
        if (isAudioFile(f))
            playlist.addIfNotAlreadyThere(f);

    listBox.updateContent();
}

juce::File PlayerGUI::PlaylistComponent::getFileAt(int index) const
{
    if (index >= 0 && index < playlist.size()) return playlist[index];
    return {};
}

juce::File PlayerGUI::PlaylistComponent::getSelectedFile() const
{
    int r = listBox.getSelectedRow();
    if (r >= 0 && r < playlist.size()) return playlist[r];
    return {};
}

bool PlayerGUI::PlaylistComponent::isAudioFile(const juce::File& f) const
{
    auto ext = f.getFileExtension().toLowerCase();
    return ext == ".wav" || ext == ".mp3" || ext == ".aiff" || ext == ".flac" || ext == ".ogg" || ext == ".m4a";
}

void PlayerGUI::PlaylistComponent::buttonClicked(juce::Button* b)
{
    if (b == &addFilesButton)
    {
        auto chooser = std::make_shared<juce::FileChooser>("Select audio files...", juce::File(), ".wav;.mp3;.aiff;.flac;.ogg;.m4a", true);
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectMultipleItems,
            [this, chooser](const juce::FileChooser& fc)
            {
                addFiles(fc.getResults());
            });
    }
}



PlayerGUI::PlayerGUI(PlayerAudio& audioEngineRef, PlaylistComponent* sharedPlaylist)
    : audioEngine(audioEngineRef),
    playlistComponent(sharedPlaylist)
{
    addAndMakeVisible(loadButton);
    loadButton.addListener(this);

    addAndMakeVisible(playPauseButton);
    playPauseButton.addListener(this);

    addAndMakeVisible(stopButton);
    stopButton.addListener(this);

    addAndMakeVisible(restartButton);
    restartButton.addListener(this);

    addAndMakeVisible(setAButton);
    setAButton.addListener(this);

    addAndMakeVisible(setBButton);
    setBButton.addListener(this);

    addAndMakeVisible(abLoopToggle);
    abLoopToggle.addListener(this);

    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0.0, 1.0, 0.001);
    volumeSlider.setValue(0.8);
    volumeSlider.addListener(this);

    addAndMakeVisible(speedSlider);
    speedSlider.setRange(0.25, 2.0, 0.01);
    speedSlider.setValue(1.0);
    speedSlider.addListener(this);

    addAndMakeVisible(muteButton);
    muteButton.addListener(this);

    addAndMakeVisible(loopButton);
    loopButton.addListener(this);

    addAndMakeVisible(titleLabel);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setText("No file", juce::dontSendNotification);

    
    addAndMakeVisible(positionSlider);
    positionSlider.setRange(0.0, 1.0, 0.001);
    positionSlider.addListener(this);
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    
    addAndMakeVisible(positionLabel);
    positionLabel.setText("0:00 / 0:00", juce::dontSendNotification);
    positionLabel.setJustificationType(juce::Justification::centred);

    
    startTimerHz(20); 

   
    thumbnail = std::make_unique<juce::AudioThumbnail>(512, audioEngine.getFormatManager(), thumbnailCache);

    startTimerHz(30);

    setSize(320, 260);
}

PlayerGUI::~PlayerGUI()
{
    stopTimer();
}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);

    if (fileLoaded && thumbnail)
    {
        auto warea = getLocalBounds().reduced(10).removeFromBottom(110);
        thumbnail->drawChannels(g, warea.reduced(2), 0.0, thumbnail->getTotalLength(), 1.0f);

        // draw playhead
        if (thumbnail->getTotalLength() > 0.0)
        {
            double pos = audioEngine.getCurrentPosition();
            double len = thumbnail->getTotalLength();
            float x = (float)juce::jmap(pos, 0.0, len, (double)warea.getX(), (double)warea.getRight());
            g.setColour(juce::Colours::red);
            g.drawLine(x, (float)warea.getY(), x, (float)warea.getBottom(), 2.0f);
        }
    }
    else
    {
        g.setColour(juce::Colours::white);
        g.drawFittedText("No audio loaded", getLocalBounds().reduced(10), juce::Justification::centred, 1);
    }
}

void PlayerGUI::resized()
{
    auto r = getLocalBounds().reduced(8);
    auto top = r.removeFromTop(26);
    titleLabel.setBounds(top);

    auto btnRow = r.removeFromTop(34);
    loadButton.setBounds(btnRow.removeFromLeft(70));
    playPauseButton.setBounds(btnRow.removeFromLeft(70));
    stopButton.setBounds(btnRow.removeFromLeft(70));
    restartButton.setBounds(btnRow.removeFromLeft(78));

    auto row2 = r.removeFromTop(30);
    setAButton.setBounds(row2.removeFromLeft(60));
    setBButton.setBounds(row2.removeFromLeft(60));
    abLoopToggle.setBounds(row2.removeFromLeft(100));
    muteButton.setBounds(row2.removeFromLeft(60));

    auto sliders = r.removeFromTop(50);
    volumeSlider.setBounds(sliders.removeFromLeft(getWidth() / 2 - 12));
    speedSlider.setBounds(sliders);

    auto bottomArea = getLocalBounds().removeFromBottom(40);
    positionSlider.setBounds(bottomArea.removeFromLeft(getWidth() * 0.8));
    positionLabel.setBounds(bottomArea);
   
}

void PlayerGUI::buttonClicked(juce::Button* b)
{
    if (b == &loadButton)
    {
        juce::File toLoad;
        if (playlistComponent)
            toLoad = playlistComponent->getSelectedFile();

        if (!toLoad.existsAsFile())
        {
            auto chooser = std::make_shared<juce::FileChooser>("Select audio file...");
            chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& fc)
                {
                    auto file = fc.getResult();
                    if (file.existsAsFile())
                    {
                        audioEngine.loadFile(file);
                        thumbnail->setSource(new juce::FileInputSource(file));
                        fileLoaded = true;
                        titleLabel.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
                    }
                });
        }
        else
        {
            audioEngine.loadFile(toLoad);
            thumbnail->setSource(new juce::FileInputSource(toLoad));
            fileLoaded = true;
            titleLabel.setText(toLoad.getFileNameWithoutExtension(), juce::dontSendNotification);
        }
    }
    else if (b == &playPauseButton)
    {
        if (audioEngine.isPlaying())
            audioEngine.pause();
        else
            audioEngine.play();

        updatePlayPauseText();
    }
    else if (b == &stopButton)
    {
        audioEngine.stop();
        updatePlayPauseText();
    }
    else if (b == &restartButton)
    {
        audioEngine.restart();
        updatePlayPauseText();
    }
    else if (b == &setAButton)
    {
        setABLoopStart();
    }
    else if (b == &setBButton)
    {
        setABLoopEnd();
    }
    else if (b == &abLoopToggle)
    {
        enableABLoop(abLoopToggle.getToggleState());
    }
    else if (b == &muteButton)
    {
        audioEngine.toggleMute();
        muteButton.setButtonText(audioEngine.isMuted() ? "Unmute" : "Mute");
    }
    else if (b == &loopButton)
    {
        audioEngine.setLooping(loopButton.getToggleState());
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* s)
{
    if (s == &volumeSlider)
    {
        if (!audioEngine.isMuted())
            audioEngine.setGain((float)volumeSlider.getValue());
    }
    else if (s == &speedSlider)
    {
        audioEngine.setSpeed(speedSlider.getValue());
    }
    else if (s == &positionSlider)
    {
        double len = audioEngine.getTotalLength();
        if (len > 0)
        {
            double newPos = positionSlider.getValue() * len;
            audioEngine.setPosition(newPos);
        }
    }
}



void PlayerGUI::updatePlayPauseText()
{
    if (audioEngine.isPlaying())
        playPauseButton.setButtonText("Pause");
    else if (audioEngine.isPaused())
        playPauseButton.setButtonText("Resume");
    else
        playPauseButton.setButtonText("Play");
}

juce::String PlayerGUI::formatTime(double s)
{
    int secs = (int)std::round(s);
    int mins = secs / 60;
    secs = secs % 60;
    return juce::String(mins) + ":" + juce::String(secs).paddedLeft('0', 2);
}

void PlayerGUI::setABLoopStart()
{
    loopStart = audioEngine.getCurrentPosition();
}

void PlayerGUI::setABLoopEnd()
{
    loopEnd = audioEngine.getCurrentPosition();
}

void PlayerGUI::enableABLoop(bool enable)
{
    isABLooping = enable;
    if (isABLooping)
        audioEngine.setPosition(loopStart);
}

void PlayerGUI::timerCallback()
{
    if (audioEngine.getTotalLength() > 0)
    {
        double pos = audioEngine.getCurrentPosition();
        double len = audioEngine.getTotalLength();

        positionSlider.setValue(pos / len, juce::dontSendNotification);

        auto formatTime = [](double seconds) -> juce::String {
            int totalSecs = (int)seconds;
            int mins = totalSecs / 60;
            int secs = totalSecs % 60;
            return juce::String(mins) + ":" + juce::String(secs).paddedLeft('0', 2);
            };

        positionLabel.setText(formatTime(pos) + " / " + formatTime(len), juce::dontSendNotification);
    }
}