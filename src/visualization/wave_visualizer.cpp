#include <cmath>
#include <algorithm>
#include "visualization/wave_visualizer.h"
#include "render/render_engine.h"

WaveVisualizer::WaveVisualizer(std::shared_ptr<RenderEngine> renderEngine)
    : Visualizer(renderEngine)
    , m_pointCount(100)
    , m_baseColor{0.0f, 0.8f, 0.8f, 1.0f}  // Cyan
    , m_beatColor{1.0f, 0.4f, 0.8f, 1.0f}  // Pink
    , m_amplitude(100.0f)
    , m_frequency(0.5f)
    , m_phase(0.0f)
    , m_beatDetected(false)
    , m_beatIntensity(0.0f)
    , m_lineThickness(3.0f)
{
    m_waveColor = m_baseColor;
}

WaveVisualizer::~WaveVisualizer() {
}

bool WaveVisualizer::initialize() {
    // Initialize wave points
    m_wavePoints.resize(m_pointCount * 2);
    
    return true;
}

void WaveVisualizer::update(
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
    
    // Get window dimensions
    int width, height;
    m_renderEngine->getViewportSize(width, height);
    
    // Calculate time-based wave parameters
    m_phase += deltaTime * 2.0f;
    if (m_phase > 2.0f * M_PI) {
        m_phase -= 2.0f * M_PI;
    }
    
    // Adjust frequency and amplitude based on audio
    float freqSum = 0.0f;
    float ampSum = 0.0f;
    
    if (!frequencyData.empty()) {
        // Average low frequencies for bass effect
        int bassBins = std::min(8, static_cast<int>(frequencyData.size() / 8));
        for (int i = 0; i < bassBins; ++i) {
            freqSum += frequencyData[i];
        }
        freqSum /= bassBins;
        
        // Average mid frequencies for amplitude effect
        int midStart = frequencyData.size() / 8;
        int midEnd = frequencyData.size() / 2;
        for (int i = midStart; i < midEnd; ++i) {
            ampSum += frequencyData[i];
        }
        ampSum /= (midEnd - midStart);
    }
    
    // Adjust frequency and amplitude
    float targetFreq = 0.5f + freqSum * 2.0f;
    float targetAmp = 50.0f + ampSum * 150.0f;
    
    // Boost on beat
    if (m_beatDetected) {
        targetAmp *= 1.5f;
    }
    
    // Smoothly transition
    m_frequency += (targetFreq - m_frequency) * std::min(1.0f, deltaTime * 5.0f);
    m_amplitude += (targetAmp - m_amplitude) * std::min(1.0f, deltaTime * 5.0f);
    
    // Update wave points
    float centerY = height * 0.5f;
    float xStep = static_cast<float>(width) / (m_pointCount - 1);
    
    for (int i = 0; i < m_pointCount; ++i) {
        float x = i * xStep;
        
        // Base sine wave
        float y = centerY + sin(m_phase + i * 0.1f * m_frequency) * m_amplitude;
        
        // Add another sine wave for complexity
        y += sin(m_phase * 0.7f + i * 0.2f) * m_amplitude * 0.3f;
        
        // Store point
        m_wavePoints[i * 2] = x;
        m_wavePoints[i * 2 + 1] = y;
    }
    
    // Update color based on beat
    float beatFactor = m_beatIntensity;
    
    for (int c = 0; c < 3; ++c) {
        m_waveColor[c] = m_baseColor[c] * (1.0f - beatFactor) + m_beatColor[c] * beatFactor;
    }
    
    // Update line thickness based on beat
    m_lineThickness = 3.0f + m_beatIntensity * 3.0f;
}

void WaveVisualizer::render() {
    if (!m_renderEngine) {
        return;
    }
    
    // Draw the wave
    m_renderEngine->drawLines(
        m_wavePoints.data(),
        m_pointCount,
        m_lineThickness,
        m_waveColor[0],
        m_waveColor[1],
        m_waveColor[2],
        m_waveColor[3]
    );
}

const char* WaveVisualizer::getName() const {
    return "Wave Visualizer";
}