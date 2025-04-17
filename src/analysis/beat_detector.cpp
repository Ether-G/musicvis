#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include "analysis/beat_detector.h"

BeatDetector::BeatDetector()
    : m_historySize(43)  // About 1 second at 44.1kHz with 1024 buffer size
    , m_sensitivity(0.3f)
    , m_currentEnergy(0.0f)
    , m_threshold(0.0f)
    , m_beatDetected(false)
    , m_cooldown(0)
    , m_cooldownPeriod(4)  // Minimum time between beats
{
}

BeatDetector::~BeatDetector() {
}

bool BeatDetector::initialize(float sensitivity) {
    m_sensitivity = std::clamp(sensitivity, 0.05f, 0.5f);
    m_energyHistory.clear();
    m_currentEnergy = 0.0f;
    m_threshold = 0.0f;
    m_beatDetected = false;
    m_cooldown = 0;
    
    std::cout << "Beat detector initialized with sensitivity: " << m_sensitivity << std::endl;
    
    return true;
}

void BeatDetector::analyzeAudio(const std::vector<float>& audioData) {
    if (audioData.empty()) {
        return;
    }
    
    // Calculate energy of the current audio buffer
    m_currentEnergy = calculateEnergy(audioData);
    
    // Update energy history
    m_energyHistory.push_back(m_currentEnergy);
    if (m_energyHistory.size() > m_historySize) {
        m_energyHistory.pop_front();
    }
    
    // Need enough history to detect beats
    if (m_energyHistory.size() < 2) {
        m_beatDetected = false;
        return;
    }
    
    // Calculate average energy as the threshold baseline
    float averageEnergy = std::accumulate(
        m_energyHistory.begin(), m_energyHistory.end(), 0.0f
    ) / m_energyHistory.size();
    
    // Calculate variance for dynamic threshold
    float variance = 0.0f;
    for (float e : m_energyHistory) {
        variance += (e - averageEnergy) * (e - averageEnergy);
    }
    variance /= m_energyHistory.size();
    
    // Set threshold as average plus some multiple of the standard deviation
    float stdDev = std::sqrt(variance);
    m_threshold = averageEnergy + m_sensitivity * stdDev;
    
    // Check if we're in cooldown period
    if (m_cooldown > 0) {
        m_cooldown--;
        m_beatDetected = false;
        return;
    }
    
    // Detect beat if energy exceeds threshold
    if (m_currentEnergy > m_threshold) {
        m_beatDetected = true;
        m_cooldown = m_cooldownPeriod;
    } else {
        m_beatDetected = false;
    }
}

float BeatDetector::calculateEnergy(const std::vector<float>& audioData) {
    float sum = 0.0f;
    
    // Calculate RMS (Root Mean Square) of the audio data
    for (float sample : audioData) {
        sum += sample * sample;
    }
    
    return std::sqrt(sum / audioData.size());
}

bool BeatDetector::isBeatDetected() const {
    return m_beatDetected;
}

float BeatDetector::getEnergy() const {
    return m_currentEnergy;
}