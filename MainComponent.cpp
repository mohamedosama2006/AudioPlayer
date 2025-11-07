#include "MainComponent.h"

MainComponent::MainComponent()
    : playerA(formatManager),
    playerB(formatManager)
{
    
    formatManager.registerBasicFormats();

    sharedPlaylist = std::make_unique<PlayerGUI::PlaylistComponent>();

    deckA = std::make_unique<PlayerGUI>(playerA, sharedPlaylist.get());
    deckB = std::make_unique<PlayerGUI>(playerB, sharedPlaylist.get());

    addAndMakeVisible(*deckA);
    addAndMakeVisible(*deckB);

    addAndMakeVisible(crossfadeSlider);
    crossfadeSlider.setRange(0.0, 1.0, 0.001);
    crossfadeSlider.setValue(0.5);

 
    addAndMakeVisible(*sharedPlaylist);

    setSize(1000, 600);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    playerA.prepareToPlay(samplesPerBlockExpected, sampleRate);
    playerB.prepareToPlay(samplesPerBlockExpected, sampleRate);

    mixerSource.addInputSource(&playerA.getAudioSourceAdapter(), false);
    mixerSource.addInputSource(&playerB.getAudioSourceAdapter(), false);

    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
   
    bufferToFill.clearActiveBufferRegion();

    juce::AudioBuffer<float> tempA(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
    juce::AudioBuffer<float> tempB(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);

    juce::AudioSourceChannelInfo infoA(&tempA, 0, bufferToFill.numSamples);
    juce::AudioSourceChannelInfo infoB(&tempB, 0, bufferToFill.numSamples);

    
    playerA.getNextAudioBlock(infoA);
    playerB.getNextAudioBlock(infoB);

    float mixValue = (float)crossfadeSlider.getValue(); 

    for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        float* out = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
        const float* inA = tempA.getReadPointer(channel);
        const float* inB = tempB.getReadPointer(channel);

        for (int i = 0; i < bufferToFill.numSamples; ++i)
        {
            out[i] = inA[i] * (1.0f - mixValue) + inB[i] * mixValue;
        }
    }
}

void MainComponent::releaseResources()
{
    mixerSource.releaseResources();
    playerA.releaseResources();
    playerB.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto r = getLocalBounds().reduced(8);
    auto left = r.removeFromLeft(260);
    sharedPlaylist->setBounds(left);

    auto top = r.removeFromTop(getHeight() / 2 - 10);
    deckA->setBounds(top.removeFromLeft(r.getWidth() / 2 - 8));
    deckB->setBounds(r); 

    crossfadeSlider.setBounds(getWidth() / 2 - 150, getHeight() - 50, 300, 24);
}