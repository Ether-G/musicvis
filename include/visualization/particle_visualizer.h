#ifndef PARTICLE_VISUALIZER_H
#define PARTICLE_VISUALIZER_H

#include <vector>
#include <array>
#include "visualization/visualizer.h"

// Particle structure
struct Particle {
    float x, y;          // Position
    float vx, vy;        // Velocity
    float size;          // Size
    float life;          // Lifetime (0.0 - 1.0)
    float maxLife;       // Maximum lifetime
    std::array<float, 4> color;  // Color (RGBA)
};

class ParticleVisualizer : public Visualizer {
public:
    ParticleVisualizer(std::shared_ptr<RenderEngine> renderEngine);
    ~ParticleVisualizer();

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
    // Spawn new particles
    void spawnParticles(
        int count, 
        float x, 
        float y, 
        float energy,
        const std::array<float, 4>& baseColor = {0.2f, 0.5f, 1.0f, 1.0f}
    );
    
    // Update particles
    void updateParticles(float deltaTime);
    
    // Particles
    std::vector<Particle> m_particles;
    
    // Maximum number of particles
    int m_maxParticles;
    
    // Emission rate (particles per second)
    float m_emissionRate;
    
    // Emission timer
    float m_emissionTimer;
    
    // Emitter position
    float m_emitterX;
    float m_emitterY;
    
    // Base color
    std::array<float, 4> m_baseColor;
    
    // Beat color
    std::array<float, 4> m_beatColor;
    
    // Alternative colors for variety
    std::vector<std::array<float, 4>> m_altColors;
    
    // Beat detected flag
    bool m_beatDetected;
    
    // Beat intensity
    float m_beatIntensity;
    
    // Last frequency data for visualization
    std::vector<float> m_lastFrequencyData;
    
    // Low frequency energy
    float m_bassEnergy;
    
    // High frequency energy
    float m_trebleEnergy;
    
    // Beat counter for pattern changes
    int m_beatCounter;
    
    // Time of last beat
    float m_lastBeatTime;
    
    // Total time elapsed
    float m_totalTime;
};

#endif // PARTICLE_VISUALIZER_H