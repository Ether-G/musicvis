#include <cmath>
#include <algorithm>
#include "visualization/bar_visualizer.h"
#include "render/render_engine.h"

BarVisualizer::BarVisualizer(std::shared_ptr<RenderEngine> renderEngine)
    : Visualizer(renderEngine)
    , m_barCount(64)
    , m_baseColor{0.2f, 0.6f, 1.0f, 1.0f}  // Blue
    , m_beatColor{1.0f, 0.2f, 0.4f, 1.0f}  // Red
    , m_barWidth(8.0f)
    , m_barSpacing(2.0f)
    , m_animationSpeed(8.0f)
    , m_beatDetected(false)
    , m_beatIntensity(0.0f)
{
}

BarVisualizer::~BarVisualizer() {
}

bool BarVisualizer::initialize() {
    // Initialize bar heights and colors
    m_barHeights.resize(m_barCount, 0.0f);
    m_targetBarHeights.resize(m_barCount, 0.0f);
    m_barColors.resize(m_barCount, m_baseColor);
    
    return true;
}

void BarVisualizer::update(
    float deltaTime,
    const std::vector<float>& audioData,
    const std::vector<float>& frequencyData,
    bool beatDetected
) {
    // Update beat detection state
    m_beatDetected = beatDetected;
    
    // Decay beat intensity
    m_beatIntensity *= std::max(0.0f, 1.0f - deltaTime * 3.0f);
    
    // Increase beat intensity on beat
    if (m_beatDetected) {
        m_beatIntensity = 1.0f;
    }
    
    // Update bar heights based on frequency data
    if (!frequencyData.empty()) {
        int binStep = std::max(1, static_cast<int>(frequencyData.size() / m_barCount));
        
        for (int i = 0; i < m_barCount; ++i) {
            int binIndex = i * binStep;
            if (binIndex < frequencyData.size()) {
                // Use logarithmic mapping for more natural visualization
                float frequency = frequencyData[binIndex];
                
                // Boost low and high frequencies for aesthetic appeal
                if (i < m_barCount / 4) {
                    // Boost bass frequencies
                    frequency *= 1.2f;
                } else if (i > m_barCount * 3 / 4) {
                    // Boost high frequencies
                    frequency *= 1.1f;
                }
                
                // Set target height
                m_targetBarHeights[i] = std::min(1.0f, frequency);
                
                // Add beat effect
                if (m_beatDetected) {
                    m_targetBarHeights[i] *= (1.0f + m_beatIntensity * 0.5f);
                }
            } else {
                m_targetBarHeights[i] = 0.0f;
            }
            
            // Smoothly animate to target height
            float diff = m_targetBarHeights[i] - m_barHeights[i];
            m_barHeights[i] += diff * std::min(1.0f, deltaTime * m_animationSpeed);
            
            // Update bar color based on height and beat
            float beatFactor = m_beatIntensity * 0.6f;
            float heightFactor = m_barHeights[i] * 0.4f;
            float colorMix = beatFactor + heightFactor;
            
            for (int c = 0; c < 3; ++c) {
                m_barColors[i][c] = m_baseColor[c] * (1.0f - colorMix) + m_beatColor[c] * colorMix;
            }
        }
    }
}

void BarVisualizer::render() {
    if (!m_renderEngine) {
        return;
    }
    
    // Get render dimensions
    int width, height;
    m_renderEngine->getViewportSize(width, height);
    
    // Calculate bar positioning
    float totalWidth = m_barCount * (m_barWidth + m_barSpacing) - m_barSpacing;
    float startX = (width - totalWidth) * 0.5f;
    float baseY = height * 0.8f;  // Position bars at the bottom part of the screen
    
    // Render each bar
    for (int i = 0; i < m_barCount; ++i) {
        float x = startX + i * (m_barWidth + m_barSpacing);
        float barHeight = height * 0.6f * m_barHeights[i]; // Maximum bar height is 60% of screen
        
        // Don't render very small bars
        if (barHeight > 1.0f) {
            m_renderEngine->drawRectangle(
                x, baseY - barHeight,
                m_barWidth, barHeight,
                m_barColors[i][0], m_barColors[i][1], m_barColors[i][2], m_barColors[i][3]
            );
        }
    }
}

const char* BarVisualizer::getName() const {
    return "Bar Visualizer";
}