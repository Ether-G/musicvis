// Improved BeatDetector implementation to be more responsive to drums and rhythmic patterns

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
    , m_cooldownPeriod(3)  // Reduced from 4 to detect beats more frequently
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
    
    // Calculate energy with emphasis on rapid changes (important for drums)
    m_currentEnergy = calculateEnergy(audioData);
    
    // Store raw audio samples for drum detection
    std::vector<float> localSamples;
    int channels = 2; // Assuming stereo
    
    // Store a subset of samples for analysis
    for (size_t i = 0; i < audioData.size(); i += channels) {
        if (localSamples.size() < 512) { // We only need a subset for analysis
            localSamples.push_back(audioData[i]);
        }
    }
    
    // Calculate derivative (rate of change) - drums have sharp transients
    float derivativeEnergy = calculateDerivativeEnergy(localSamples);
    
    // Combine energies with emphasis on derivative for drums
    float combinedEnergy = m_currentEnergy * 0.5f + derivativeEnergy * 0.5f;
    
    // Update energy history
    m_energyHistory.push_back(combinedEnergy);
    if (m_energyHistory.size() > m_historySize) {
        m_energyHistory.pop_front();
    }
    
    // Need enough history to detect beats
    if (m_energyHistory.size() < 4) {
        m_beatDetected = false;
        return;
    }
    
    // Calculate local average energy with exponential decay weighting
    // This puts more emphasis on recent energy levels
    float totalWeight = 0.0f;
    float weightedSum = 0.0f;
    
    int count = m_energyHistory.size();
    for (int i = 0; i < count; i++) {
        // Exponential weighting gives more importance to recent samples
        float weight = std::exp(0.5f * (i - count + 1));
        weightedSum += m_energyHistory[i] * weight;
        totalWeight += weight;
    }
    
    float averageEnergy = weightedSum / totalWeight;
    
    // Calculate variance for dynamic threshold
    float variance = 0.0f;
    for (float e : m_energyHistory) {
        variance += (e - averageEnergy) * (e - averageEnergy);
    }
    variance /= m_energyHistory.size();
    
    // Calculate dynamic threshold
    float stdDev = std::sqrt(variance);
    float dynamicSensitivity = m_sensitivity;
    
    // Adjust sensitivity based on audio characteristics
    // Lower sensitivity for high energy sections to avoid false positives
    if (averageEnergy > 0.1f) {
        dynamicSensitivity *= 1.2f;
    }
    
    m_threshold = averageEnergy + dynamicSensitivity * stdDev;
    
    // Apply minimum threshold to avoid detecting noise
    float minThreshold = 0.01f;
    m_threshold = std::max(m_threshold, minThreshold);
    
    // Check if we're in cooldown period
    if (m_cooldown > 0) {
        m_cooldown--;
        m_beatDetected = false;
        return;
    }
    
    // Enhanced beat detection logic
    // Detect beat if:
    // 1. Current energy exceeds threshold, AND
    // 2. Current energy is significantly higher than previous energy (sharp rise)
    // 3. AND current energy is higher than the local average
    
    float previousEnergy = m_energyHistory[m_energyHistory.size() - 2];
    float energyRatio = combinedEnergy / (previousEnergy + 0.01f); // Avoid division by zero
    
    bool isEnergyRising = energyRatio > 1.1f; // 10% increase
    bool isAboveThreshold = combinedEnergy > m_threshold;
    bool isAboveAverage = combinedEnergy > averageEnergy * 1.1f;
    
    if (isAboveThreshold && isEnergyRising && isAboveAverage) {
        m_beatDetected = true;
        m_cooldown = m_cooldownPeriod;
        
        // Debug output
        std::cout << "Beat detected! Energy: " << combinedEnergy 
                 << " Threshold: " << m_threshold 
                 << " Ratio: " << energyRatio << std::endl;
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

float BeatDetector::calculateDerivativeEnergy(const std::vector<float>& samples) {
    if (samples.size() < 2) {
        return 0.0f;
    }
    
    float sum = 0.0f;
    
    // Calculate the energy of the first derivative (rate of change)
    // This is very effective for detecting drum hits which have sharp transients
    for (size_t i = 1; i < samples.size(); i++) {
        float derivative = samples[i] - samples[i-1];
        sum += derivative * derivative;
    }
    
    return std::sqrt(sum / (samples.size() - 1));
}

bool BeatDetector::isBeatDetected() const {
    return m_beatDetected;
}

float BeatDetector::getEnergy() const {
    return m_currentEnergy;
}