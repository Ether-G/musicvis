#ifndef BEAT_DETECTOR_H
#define BEAT_DETECTOR_H

#include <vector>
#include <deque>

class BeatDetector {
public:
    BeatDetector();
    ~BeatDetector();

    // Initialize the beat detector with a given sensitivity
    bool initialize(float sensitivity);
    
    // Analyze audio data for beats
    void analyzeAudio(const std::vector<float>& audioData);
    
    // Check if a beat is detected
    bool isBeatDetected() const;
    
    // Get the current energy level
    float getEnergy() const;

private:
    // Calculate energy (RMS) of the audio data
    float calculateEnergy(const std::vector<float>& audioData);
    
    // History of energy values for dynamic threshold
    std::deque<float> m_energyHistory;
    
    // Number of history values to keep
    int m_historySize;
    
    // Sensitivity value (0.0 - 1.0)
    float m_sensitivity;
    
    // Current energy value
    float m_currentEnergy;
    
    // Beat detection threshold
    float m_threshold;
    
    // Is a beat currently detected
    bool m_beatDetected;
    
    // Cooldown counter to prevent double triggers
    int m_cooldown;
    
    // Cooldown period in samples
    int m_cooldownPeriod;
};

#endif // BEAT_DETECTOR_H