#pragma once
#include <JuceHeader.h>


class PlayerAudio : public juce::AudioSource,
    public juce::ChangeListener
{
public:
    PlayerAudio(juce::AudioFormatManager& formatManagerRef);
    ~PlayerAudio() override;

   
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    
    void loadFile(const juce::File& file);
    void play();
    void pause();
    void stop();
    void restart();
    void goToStart();
    void goToEnd();

    void setGain(float g);
    void setSpeed(double ratio);
    void setPosition(double pos);
    double getCurrentPosition() const;
    double getTotalLength() const;
    bool isPlaying() const;
    bool isPaused() const { return paused; }
    bool isMuted() const { return muted; }

    juce::String getMetadata() const;
    juce::AudioFormatManager& getFormatManager() { return formatManager; }
    juce::AudioSource& getAudioSourceAdapter() { return resamplingSource; }

    
    void toggleMute();
    void setLooping(bool shouldLoop);

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    float getOutputSample(int channel, int sampleIndex)
    {
        if (readerSource != nullptr)
        {
            auto* reader = readerSource->getAudioFormatReader();
            if (reader != nullptr)
                return 0.0f; 
        }
        return 0.0f;
    }

private:
    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resamplingSource{ &transportSource, false, 2 };

    bool paused = false;
    bool muted = false;
    float currentGain = 1.0f;

    juce::String metadata;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerAudio)
};