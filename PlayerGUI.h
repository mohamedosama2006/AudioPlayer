#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::Timer
{
public:
    class PlaylistComponent : public juce::Component,
        public juce::ListBoxModel,
        public juce::Button::Listener
    {
    public:
        PlaylistComponent();
        ~PlaylistComponent() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

       
        int getNumRows() override;
        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;
        void buttonClicked(juce::Button* b)override;

       
        void addFiles(const juce::Array<juce::File>& files);
        juce::File getFileAt(int index) const;
        int getNumFiles() const { return playlist.size(); }

        
        juce::File getSelectedFile() const;

    private:
        juce::ListBox listBox;
        juce::Array<juce::File> playlist;
        juce::TextButton addFilesButton;

        bool isAudioFile(const juce::File& f) const;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
    };

    PlayerGUI(PlayerAudio& audioEngine, PlaylistComponent* sharedPlaylist = nullptr);
    ~PlayerGUI() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void buttonClicked(juce::Button* b) override;
    void sliderValueChanged(juce::Slider* s) override;
    void timerCallback() override;

    juce::AudioSource* getAudioSource() { return &audioEngine.getAudioSourceAdapter(); }

    PlayerAudio& getPlayerAudio() { return audioEngine; }

    void setABLoopStart();
    void setABLoopEnd();
    void enableABLoop(bool enable);

private:
    PlayerAudio& audioEngine;

    PlaylistComponent* playlistComponent = nullptr;

    
    juce::TextButton loadButton{ "Load" };
    juce::TextButton playPauseButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton setAButton{ "Set A" };
    juce::TextButton setBButton{ "Set B" };
    juce::ToggleButton abLoopToggle{ "AB Loop" };
    juce::Slider volumeSlider;
    juce::Slider speedSlider;
    juce::ToggleButton muteButton{ "Mute" };
    juce::ToggleButton loopButton{ "Loop" };
    juce::AudioThumbnailCache thumbnailCache{ 5 };
    std::unique_ptr<juce::AudioThumbnail> thumbnail;
    juce::Label titleLabel;

   
    double loopStart = 0.0;
    double loopEnd = 0.0;
    bool isABLooping = false;

  
    bool fileLoaded = false;
    double currentPos = 0.0;

    
    void updatePlayPauseText();
    juce::String formatTime(double s);
    juce::Slider positionSlider;
    juce::Label positionLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};