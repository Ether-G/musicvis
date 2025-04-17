#ifndef BAR_VISUALIZER_H
#define BAR_VISUALIZER_H

#include <vector>
#include <array>
#include "visualization/visualizer.h"

class BarVisualizer : public Visualizer {
public:
    BarVisualizer(std::shared_ptr<RenderEngine> renderEngine);
    ~BarVisualizer();

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
    // Number of bars to display
    int m_barCount;
    
    // Bar heights
    std::vector<float> m_barHeights;
    
    // Target bar heights (for smooth animation)
    std::vector<float> m_targetBarHeights;
    
    // Bar colors
    std::vector<std::array<float, 4>> m_barColors;
    
    // Base color
    std::array<float, 4> m_baseColor;
    
    // Beat color
    std::array<float, 4> m_beatColor;
    
    // Bar width
    float m_barWidth;
    
    // Bar spacing
    float m_barSpacing;
    
    // Animation speed
    float m_animationSpeed;
    
    // Beat detected flag
    bool m_beatDetected;
    
    // Beat intensity
    float m_beatIntensity;
};

#endif // BAR_VISUALIZER_H