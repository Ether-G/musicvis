#include <cmath>
#include <iostream>
#include <fstream>
#include <cstring>
#include "audio/audio_buffer.h"

AudioBuffer::AudioBuffer()
    : m_position(0)
    , m_sampleRate(44100)
    , m_channelCount(2)
{
}

AudioBuffer::~AudioBuffer() {
}

bool AudioBuffer::loadFromFile(const std::string& filePath) {
#ifdef USE_LIBSNDFILE
    std::cout << "Using libsndfile to load: " << filePath << std::endl;
    
    // Open sound file
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));
    
    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfInfo);
    if (!file) {
        std::cerr << "Error opening sound file: " << sf_strerror(NULL) << std::endl;
        return false;
    }
    
    // Set sample rate and channel count
    m_sampleRate = sfInfo.samplerate;
    m_channelCount = sfInfo.channels;
    
    // Read the entire file
    std::vector<float> buffer(sfInfo.frames * sfInfo.channels);
    sf_count_t count = sf_readf_float(file, buffer.data(), sfInfo.frames);
    
    // Close the file
    sf_close(file);
    
    if (count != sfInfo.frames) {
        std::cerr << "Error reading sound file: " << filePath << std::endl;
        return false;
    }
    
    // Store the audio data
    std::lock_guard<std::mutex> lock(m_mutex);
    m_audioData = buffer;
    m_position = 0;
    
    std::cout << "Loaded audio file: " << filePath << std::endl;
    std::cout << "Sample rate: " << m_sampleRate << ", Channels: " << m_channelCount << std::endl;
    std::cout << "Duration: " << float(sfInfo.frames) / m_sampleRate << " seconds" << std::endl;
    
    return true;
#else
    std::cout << "USE_LIBSNDFILE not defined, using sine wave" << std::endl;
    
    // For simplicity, we'll just simulate loading a file
    // In a real application, you would use libsndfile or similar
    std::cout << "Note: This is a simplified implementation." << std::endl;
    std::cout << "In a real application, use libsndfile to load audio files." << std::endl;
    
    // Create some test data - a simple sine wave
    const float frequency = 440.0f; // 440 Hz A note
    const float amplitude = 0.5f;
    const float duration = 5.0f; // 5 seconds
    
    size_t numSamples = static_cast<size_t>(m_sampleRate * duration * m_channelCount);
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_audioData.resize(numSamples);
    
    for (size_t i = 0; i < numSamples; i += m_channelCount) {
        float t = static_cast<float>(i / m_channelCount) / m_sampleRate;
        float value = amplitude * sin(2.0f * M_PI * frequency * t);
        
        // Fill all channels with the same value
        for (int c = 0; c < m_channelCount; ++c) {
            m_audioData[i + c] = value;
        }
    }
    
    m_position = 0;
    
    std::cout << "Created test audio: " << duration << " seconds, "
              << m_sampleRate << " Hz, " << m_channelCount << " channels" << std::endl;
    
    return true;
#endif
}

std::vector<float> AudioBuffer::getSamples(size_t numSamples) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_position >= m_audioData.size()) {
        // End of audio
        return std::vector<float>();
    }
    
    size_t samplesAvailable = m_audioData.size() - m_position;
    size_t samplesToReturn = std::min(numSamples, samplesAvailable);
    
    std::vector<float> samples(samplesToReturn);
    
    // Copy samples
    memcpy(
        samples.data(),
        m_audioData.data() + m_position,
        samplesToReturn * sizeof(float)
    );
    
    // Update position
    m_position += samplesToReturn;
    
    return samples;
}

void AudioBuffer::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_position = 0;
}

int AudioBuffer::getSampleRate() const {
    return m_sampleRate;
}

int AudioBuffer::getChannelCount() const {
    return m_channelCount;
}