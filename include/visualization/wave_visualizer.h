#ifndef WAVE_VISUALIZER_H
#define WAVE_VISUALIZER_H

#include <vector>
#include <array>
#include "visualization/visualizer.h"

class WaveVisualizer : public Visualizer {
public:
    WaveVisualizer(std::shared_ptr<RenderEngine> renderEngine);
    ~WaveVisualizer();

    // Initialize the visualizer
    bool initialize() override;
    
    // Update visualizer state with new audio data
    void update(
        float deltaTime,
        const std::vector<float>& audioData,
        const std::vector<float>& frequencyData,
        bool beatDetected
    ) override;
    
    // Render the visualization
    void render() override;
    
    // Get the visualizer name
    const char* getName() const override;

private:
    // Number of points in the wave
    int m_pointCount;
    
    // Wave points (x, y)
    std::vector<float> m_wavePoints;
    
    // Wave color
    std::array<float, 4> m_waveColor;
    
    // Base color when no beat is detected
    std::array<float, 4> m_baseColor;
    
    // Beat color when a beat is detected
    std::array<float, 4> m_beatColor;
    
    // Wave amplitude
    float m_amplitude;
    
    // Wave frequency
    float m_frequency;
    
    // Wave phase
    float m_phase;
    
    // Beat detected flag
    bool m_beatDetected;
    
    // Beat intensity
    float m_beatIntensity;
    
    // Line thickness
    float m_lineThickness;
};

#endif // WAVE_VISUALIZER_H