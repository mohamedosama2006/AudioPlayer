#pragma once

#include <JuceHeader.h>
#include "PlayerGUI.h"
#include "PlayerAudio.h"


class MainComponent : public juce::AudioAppComponent,
    public juce::AudioSource
{
public:
    MainComponent();
    ~MainComponent() override;

    
    void paint(juce::Graphics&) override;
    void resized() override;

    
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

private:
    juce::AudioFormatManager formatManager;

    std::unique_ptr<PlayerGUI::PlaylistComponent> sharedPlaylist;

    PlayerAudio playerA;
    PlayerAudio playerB;

    
    std::unique_ptr<PlayerGUI> deckA;
    std::unique_ptr<PlayerGUI> deckB;

    
    juce::MixerAudioSource mixerSource;
    juce::Slider crossfadeSlider;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};