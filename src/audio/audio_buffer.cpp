#include <cmath>
#include <iostream>
#include <fstream>
#include <cstring>
#include "audio/audio_buffer.h"

// We'll use a simple implementation for now, in a real application
// you would use a library like libsndfile to handle various audio formats

AudioBuffer::AudioBuffer()
    : m_position(0)
    , m_sampleRate(44100)
    , m_channelCount(2)
{
}

AudioBuffer::~AudioBuffer() {
}

bool AudioBuffer::loadFromFile(const std::string& filePath) {
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