#include <cmath>
#include <random>
#include <algorithm>
#include "visualization/particle_visualizer.h"
#include "render/render_engine.h"

// Random number generator
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> dis(0.0f, 1.0f);

// Helper to get a random float
float getRandomFloat() {
    return dis(gen);
}

ParticleVisualizer::ParticleVisualizer(std::shared_ptr<RenderEngine> renderEngine)
    : Visualizer(renderEngine)
    , m_maxParticles(500)
    , m_emissionRate(100.0f)
    , m_emissionTimer(0.0f)
    , m_emitterX(0.0f)
    , m_emitterY(0.0f)
    , m_baseColor{0.2f, 0.5f, 1.0f, 1.0f}  // Blue
    , m_beatColor{1.0f, 0.3f, 0.7f, 1.0f}  // Pink
    , m_beatDetected(false)
    , m_beatIntensity(0.0f)
    , m_bassEnergy(0.0f)
    , m_trebleEnergy(0.0f)
{
}

ParticleVisualizer::~ParticleVisualizer() {
}

bool ParticleVisualizer::initialize() {
    // Initialize particles
    m_particles.reserve(m_maxParticles);
    
    // Get window dimensions
    int width, height;
    m_renderEngine->getViewportSize(width, height);
    
    // Set emitter position
    m_emitterX = width * 0.5f;
    m_emitterY = height * 0.5f;
    
    return true;
}

void ParticleVisualizer::update(
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
    
    // Calculate audio energy in different frequency bands
    if (!frequencyData.empty()) {
        // Save frequency data for visualization
        m_lastFrequencyData = frequencyData;
        
        // Calculate low frequency (bass) energy
        int bassBins = std::min(8, static_cast<int>(frequencyData.size() / 8));
        float bassSum = 0.0f;
        for (int i = 0; i < bassBins; ++i) {
            bassSum += frequencyData[i];
        }
        m_bassEnergy = bassSum / bassBins;
        
        // Calculate high frequency (treble) energy
        int trebleStart = static_cast<int>(frequencyData.size() * 0.75f);
        int trebleBins = frequencyData.size() - trebleStart;
        float trebleSum = 0.0f;
        for (int i = trebleStart; i < frequencyData.size(); ++i) {
            trebleSum += frequencyData[i];
        }
        m_trebleEnergy = trebleSum / trebleBins;
    }
    
    // Update particles
    updateParticles(deltaTime);
    
    // Get window dimensions
    int width, height;
    m_renderEngine->getViewportSize(width, height);
    
    // Emit particles based on audio
    m_emissionTimer += deltaTime;
    float emissionInterval = 1.0f / (m_emissionRate * (1.0f + m_bassEnergy * 2.0f));
    
    while (m_emissionTimer >= emissionInterval) {
        m_emissionTimer -= emissionInterval;
        
        // Determine spawn position based on frequency data
        float spawnX = width * 0.5f;
        float spawnY = height * 0.5f;
        
        if (!m_lastFrequencyData.empty()) {
            // Use frequency data to determine emission pattern
            int binIndex = static_cast<int>(getRandomFloat() * m_lastFrequencyData.size());
            float binValue = m_lastFrequencyData[binIndex];
            
            // Map bin index to angle
            float angle = (static_cast<float>(binIndex) / m_lastFrequencyData.size()) * 2.0f * M_PI;
            
            // Use energy to determine distance from center
            float distance = (height * 0.4f) * binValue;
            
            spawnX = width * 0.5f + cos(angle) * distance;
            spawnY = height * 0.5f + sin(angle) * distance;
        }
        
        // Spawn particles
        int particlesToSpawn = 1;
        if (m_beatDetected) {
            particlesToSpawn = 5;
        }
        
        spawnParticles(
            particlesToSpawn,
            spawnX,
            spawnY,
            m_bassEnergy + m_trebleEnergy * 0.5f
        );
    }
}

void ParticleVisualizer::render() {
    if (!m_renderEngine) {
        return;
    }
    
    // Draw each particle
    for (const Particle& particle : m_particles) {
        m_renderEngine->drawCircle(
            particle.x,
            particle.y,
            particle.size,
            8,  // Segments
            particle.color[0],
            particle.color[1],
            particle.color[2],
            particle.color[3] * (particle.life / particle.maxLife)  // Fade out
        );
    }
}

const char* ParticleVisualizer::getName() const {
    return "Particle Visualizer";
}

void ParticleVisualizer::spawnParticles(int count, float x, float y, float energy) {
    // Get window dimensions
    int width, height;
    m_renderEngine->getViewportSize(width, height);
    
    // Calculate color based on beat and energy
    std::array<float, 4> particleColor;
    float beatFactor = m_beatIntensity;
    float energyFactor = std::min(1.0f, energy);
    float colorMix = std::max(beatFactor, energyFactor * 0.7f);
    
    for (int c = 0; c < 3; ++c) {
        particleColor[c] = m_baseColor[c] * (1.0f - colorMix) + m_beatColor[c] * colorMix;
    }
    particleColor[3] = 1.0f;
    
    for (int i = 0; i < count; ++i) {
        // Don't exceed max particles
        if (m_particles.size() >= m_maxParticles) {
            break;
        }
        
        Particle p;
        p.x = x + (getRandomFloat() - 0.5f) * 10.0f;
        p.y = y + (getRandomFloat() - 0.5f) * 10.0f;
        
        // Velocity based on energy and position
        float velMagnitude = 50.0f + energy * 100.0f;
        float dirX = p.x - width * 0.5f;
        float dirY = p.y - height * 0.5f;
        float dirLength = std::sqrt(dirX * dirX + dirY * dirY);
        
        if (dirLength > 0.0001f) {
            dirX /= dirLength;
            dirY /= dirLength;
        } else {
            dirX = getRandomFloat() * 2.0f - 1.0f;
            dirY = getRandomFloat() * 2.0f - 1.0f;
        }
        
        p.vx = dirX * velMagnitude * (0.5f + getRandomFloat() * 0.5f);
        p.vy = dirY * velMagnitude * (0.5f + getRandomFloat() * 0.5f);
        
        // Size based on energy
        p.size = 2.0f + energy * 8.0f * getRandomFloat();
        
        // Lifetime based on size
        p.maxLife = 1.0f + getRandomFloat() * 2.0f;
        p.life = p.maxLife;
        
        // Color with slight variation
        p.color = particleColor;
        for (int c = 0; c < 3; ++c) {
            p.color[c] += (getRandomFloat() - 0.5f) * 0.2f;
            p.color[c] = std::clamp(p.color[c], 0.0f, 1.0f);
        }
        
        m_particles.push_back(p);
    }
}

void ParticleVisualizer::updateParticles(float deltaTime) {
    // Update each particle
    for (auto& p : m_particles) {
        // Update position
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
        
        // Apply gravity
        p.vy += 30.0f * deltaTime;
        
        // Apply drag
        p.vx *= 0.99f;
        p.vy *= 0.99f;
        
        // Update lifetime
        p.life -= deltaTime;
    }
    
    // Remove dead particles
    m_particles.erase(
        std::remove_if(
            m_particles.begin(),
            m_particles.end(),
            [](const Particle& p) { return p.life <= 0.0f; }
        ),
        m_particles.end()
    );
}