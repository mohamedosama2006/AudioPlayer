#include "PlayerAudio.h"

PlayerAudio::PlayerAudio(juce::AudioFormatManager& fm)
    : formatManager(fm)
{
    transportSource.addChangeListener(this);
}


PlayerAudio::~PlayerAudio()
{
    transportSource.removeChangeListener(this);
    transportSource.setSource(nullptr);
    releaseResources();
}

void PlayerAudio::setLooping(bool shouldLoop)
{
    transportSource.setLooping(shouldLoop);
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    resamplingSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    resamplingSource.getNextAudioBlock(bufferToFill);

    
    bufferToFill.buffer->applyGain(bufferToFill.startSample, bufferToFill.numSamples, currentGain);
}

void PlayerAudio::releaseResources()
{
    resamplingSource.releaseResources();
    transportSource.releaseResources();
}

void PlayerAudio::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        metadata = {};
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset(newSource.release());

        
        juce::StringPairArray md = reader->metadataValues;
        juce::String title = md.getValue("title", "");
        juce::String artist = md.getValue("artist", "");
        if (title.isNotEmpty() || artist.isNotEmpty())
            metadata = (title.isNotEmpty() ? ("Title: " + title + "\n") : "") + (artist.isNotEmpty() ? ("Artist: " + artist + "\n") : "");
        metadata += "Duration: " + juce::String(reader->lengthInSamples / reader->sampleRate, 2) + "s";
    }
}

void PlayerAudio::play()
{
    paused = false;
    transportSource.start();
}

void PlayerAudio::pause()
{
    if (transportSource.isPlaying())
    {
        paused = true;
        transportSource.stop();
    }
}

void PlayerAudio::stop()
{
    paused = false;
    transportSource.stop();
    transportSource.setPosition(0.0);
}

void PlayerAudio::restart()
{
    transportSource.setPosition(0.0);
    if (!paused)
        transportSource.start();
}

void PlayerAudio::goToStart()
{
    transportSource.setPosition(0.0);
}

void PlayerAudio::goToEnd()
{
    auto len = transportSource.getLengthInSeconds();
    if (len > 0.1)
        transportSource.setPosition(len - 0.05);
}

void PlayerAudio::setGain(float g)
{
    currentGain = juce::jlimit(0.0f, 2.0f, g);
}

void PlayerAudio::setSpeed(double ratio)
{
    resamplingSource.setResamplingRatio(ratio);
}

void PlayerAudio::setPosition(double pos)
{
    transportSource.setPosition(pos);
}

double PlayerAudio::getCurrentPosition() const
{
    return transportSource.getCurrentPosition();
}

double PlayerAudio::getTotalLength() const
{
    return transportSource.getLengthInSeconds();
}

bool PlayerAudio::isPlaying() const
{
    return transportSource.isPlaying();
}

juce::String PlayerAudio::getMetadata() const
{
    return metadata;
}

void PlayerAudio::toggleMute()
{
    if (!muted)
    {
        muted = true;
        transportSource.setGain(0.0f);
    }
    else
    {
        muted = false;
        transportSource.setGain(currentGain);
    }
}

void PlayerAudio::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (!transportSource.isPlaying() && !paused)
        {
           
        }
    }
}
