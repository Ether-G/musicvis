#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include <string>
#include <vector>
#include <mutex>

class AudioBuffer {
public:
    AudioBuffer();
    ~AudioBuffer();

    // Load audio data from a file
    bool loadFromFile(const std::string& filePath);
    
    // Get a chunk of samples for playback or processing
    std::vector<float> getSamples(size_t numSamples);
    
    // Reset playback position to the beginning
    void reset();
    
    // Get the sample rate
    int getSampleRate() const;
    
    // Get the number of channels
    int getChannelCount() const;

private:
    // Audio data storage
    std::vector<float> m_audioData;
    
    // Current position in the buffer
    size_t m_position;
    
    // Sample rate
    int m_sampleRate;
    
    // Channel count
    int m_channelCount;
    
    // Mutex for thread safety
    std::mutex m_mutex;
};

#endif // AUDIO_BUFFER_H