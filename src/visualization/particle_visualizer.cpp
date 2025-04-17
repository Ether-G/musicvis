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
    , m_maxParticles(800)  // Increased from 500 for more visual density
    , m_emissionRate(150.0f)  // Increased from 100.0f for more particles
    , m_emissionTimer(0.0f)
    , m_emitterX(0.0f)
    , m_emitterY(0.0f)
    , m_baseColor{0.2f, 0.5f, 1.0f, 1.0f}  // Blue
    , m_beatColor{1.0f, 0.3f, 0.7f, 1.0f}  // Pink
    , m_beatDetected(false)
    , m_beatIntensity(0.0f)
    , m_bassEnergy(0.0f)
    , m_trebleEnergy(0.0f)
    , m_beatCounter(0)
    , m_lastBeatTime(0.0f)
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
    
    // Additional colors for variety
    m_altColors.push_back({1.0f, 0.2f, 0.2f, 1.0f}); // Red
    m_altColors.push_back({0.2f, 1.0f, 0.2f, 1.0f}); // Green
    m_altColors.push_back({1.0f, 0.7f, 0.2f, 1.0f}); // Orange
    m_altColors.push_back({0.7f, 0.2f, 1.0f, 1.0f}); // Purple
    
    // Save time for time-based effects
    m_totalTime = 0.0f;
    
    return true;
}

void ParticleVisualizer::update(
    float deltaTime,
    const std::vector<float>& audioData,
    const std::vector<float>& frequencyData,
    bool beatDetected
) {
    // Update time tracker
    m_totalTime += deltaTime;
    
    // Update beat detection state
    bool previousBeatState = m_beatDetected;
    m_beatDetected = beatDetected;
    
    // Detect if this is a new beat
    bool newBeat = !previousBeatState && m_beatDetected;
    
    // Track beat timing
    if (newBeat) {
        float timeSinceLastBeat = m_totalTime - m_lastBeatTime;
        m_lastBeatTime = m_totalTime;
        m_beatCounter++;
        
        // Cycle through color schemes on every 4th beat
        if (m_beatCounter % 4 == 0) {
            // Swap base and beat colors for variety
            std::swap(m_baseColor, m_beatColor);
        }
    }
    
    // Decay beat intensity with more responsive curve
    m_beatIntensity *= std::max(0.0f, 1.0f - deltaTime * 4.0f);  // Increased from 3.0f
    
    // Increase beat intensity on beat with stronger pulse
    if (m_beatDetected) {
        m_beatIntensity = 1.8f;  // Increased from 1.0f for more dramatic effect
    }
    
    // Calculate audio energy in different frequency bands
    if (!frequencyData.empty()) {
        // Save frequency data for visualization
        m_lastFrequencyData = frequencyData;
        
        // Calculate low frequency (bass) energy - lower range for more accuracy
        int bassBins = std::min(6, static_cast<int>(frequencyData.size() / 10));
        float bassSum = 0.0f;
        for (int i = 0; i < bassBins; ++i) {
            bassSum += frequencyData[i];
        }
        // Smoother bass energy transition but more responsive
        m_bassEnergy = m_bassEnergy * 0.7f + (bassSum / bassBins) * 0.3f;
        
        // Apply more boost to bass for visualization
        m_bassEnergy = std::min(1.0f, m_bassEnergy * 1.5f);
        
        // Calculate mid frequency energy
        int midStart = static_cast<int>(frequencyData.size() * 0.1f);
        int midEnd = static_cast<int>(frequencyData.size() * 0.5f);
        float midSum = 0.0f;
        for (int i = midStart; i < midEnd; ++i) {
            midSum += frequencyData[i];
        }
        float midEnergy = midSum / (midEnd - midStart);
        
        // Calculate high frequency (treble) energy
        int trebleStart = static_cast<int>(frequencyData.size() * 0.6f);
        int trebleBins = frequencyData.size() - trebleStart;
        float trebleSum = 0.0f;
        for (int i = trebleStart; i < frequencyData.size(); ++i) {
            trebleSum += frequencyData[i];
        }
        // Smoother treble energy transition
        m_trebleEnergy = m_trebleEnergy * 0.7f + (trebleSum / trebleBins) * 0.3f;
        
        // More boost to treble energy for visualization
        m_trebleEnergy = std::min(1.0f, m_trebleEnergy * 1.4f);
    }
    
    // Update particles
    updateParticles(deltaTime);
    
    // Get window dimensions
    int width, height;
    m_renderEngine->getViewportSize(width, height);
    
    // Emit particles based on audio
    // More responsive emission rate based on audio energy
    float baseEmissionRate = m_emissionRate * (1.0f + m_beatIntensity);
    float energyFactor = m_bassEnergy * 2.0f + m_trebleEnergy * 0.5f;
    float currentEmissionRate = baseEmissionRate * (1.0f + energyFactor);
    
    m_emissionTimer += deltaTime;
    float emissionInterval = 1.0f / currentEmissionRate;
    
    // Multi-emitter behavior based on beat pattern
    int numEmitters = 1;
    if (m_beatCounter % 8 < 4) {
        numEmitters = 1; // Single central emitter
    } else {
        numEmitters = 3; // Three emitters
    }
    
    while (m_emissionTimer >= emissionInterval) {
        m_emissionTimer -= emissionInterval;
        
        for (int emitter = 0; emitter < numEmitters; emitter++) {
            // Calculate emitter position based on beat pattern
            float spawnX, spawnY;
            
            if (numEmitters == 1) {
                // Single central emitter with circular motion
                float angle = m_totalTime * 0.5f;
                float radius = width * 0.2f * m_bassEnergy;
                spawnX = width * 0.5f + cos(angle) * radius;
                spawnY = height * 0.5f + sin(angle) * radius;
            } else {
                // Multiple emitters in different positions
                float emitterAngle = (2.0f * M_PI * emitter) / numEmitters + m_totalTime * 0.2f;
                float emitterDist = width * 0.3f;
                spawnX = width * 0.5f + cos(emitterAngle) * emitterDist;
                spawnY = height * 0.5f + sin(emitterAngle) * emitterDist;
            }
            
            // Frequency-based emission pattern
            if (!m_lastFrequencyData.empty()) {
                // Use different frequency bands for each emitter
                int binOffset = (emitter * m_lastFrequencyData.size() / numEmitters) % m_lastFrequencyData.size();
                int binIndex = (binOffset + static_cast<int>(getRandomFloat() * m_lastFrequencyData.size() / 4)) % m_lastFrequencyData.size();
                float binValue = m_lastFrequencyData[binIndex];
                
                // Add some randomness based on frequency
                float angle = (static_cast<float>(binIndex) / m_lastFrequencyData.size()) * 2.0f * M_PI;
                float distance = (height * 0.2f) * binValue;
                
                spawnX += cos(angle) * distance * getRandomFloat();
                spawnY += sin(angle) * distance * getRandomFloat();
            }
            
            // Spawn more particles during beat
            int particlesToSpawn = 1;
            if (m_beatDetected) {
                particlesToSpawn = 8; // Increased from 5
            } else if (m_bassEnergy > 0.6f) {
                particlesToSpawn = 4; // Spawn more on bass even without beat
            }
            
            // Get appropriate color for this emitter
            std::array<float, 4> emitterColor;
            if (emitter == 0) {
                emitterColor = m_baseColor;
            } else if (emitter == 1) {
                emitterColor = m_beatColor;
            } else {
                // Use alternate colors for additional emitters
                emitterColor = m_altColors[emitter % m_altColors.size()];
            }
            
            // Spawn particles with specified emitter color
            spawnParticles(
                particlesToSpawn,
                spawnX,
                spawnY,
                m_bassEnergy + m_trebleEnergy * 0.5f,
                emitterColor
            );
        }
    }
}

void ParticleVisualizer::render() {
    if (!m_renderEngine) {
        return;
    }
    
    // Draw each particle with potential glow effect during beats
    for (const Particle& particle : m_particles) {
        // Normal particle
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
        
        // Add glow effect for larger particles during beats
        if (m_beatIntensity > 0.5f && particle.size > 4.0f) {
            // Draw a larger, more transparent circle for glow
            m_renderEngine->drawCircle(
                particle.x,
                particle.y,
                particle.size * 1.8f,  // Larger size for glow
                12,  // More segments for smoother glow
                particle.color[0],
                particle.color[1],
                particle.color[2],
                particle.color[3] * 0.3f * m_beatIntensity * (particle.life / particle.maxLife)  // Transparent glow
            );
        }
    }
}

const char* ParticleVisualizer::getName() const {
    return "Particle Visualizer";
}

void ParticleVisualizer::spawnParticles(int count, float x, float y, float energy, const std::array<float, 4>& baseColor) {
    // Get window dimensions
    int width, height;
    m_renderEngine->getViewportSize(width, height);
    
    // Calculate color based on beat and energy
    std::array<float, 4> particleColor;
    float beatFactor = m_beatIntensity;
    float energyFactor = std::min(1.0f, energy);
    float colorMix = std::max(beatFactor, energyFactor * 0.7f);
    
    for (int c = 0; c < 3; ++c) {
        particleColor[c] = baseColor[c] * (1.0f - colorMix) + m_beatColor[c] * colorMix;
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
        
        // Velocity based on energy and beat state
        float velMagnitude;
        if (m_beatDetected) {
            velMagnitude = 150.0f + energy * 200.0f; // Much faster on beats
        } else {
            velMagnitude = 50.0f + energy * 100.0f;
        }
        
        // Create more interesting patterns based on beat count
        float dirX, dirY;
        
        // Different emission patterns based on beat count
        switch (m_beatCounter % 4) {
            case 0: // Circular pattern
                {
                    float angle = getRandomFloat() * 2.0f * M_PI;
                    dirX = cos(angle);
                    dirY = sin(angle);
                }
                break;
                
            case 1: // Radial burst from center
                {
                    dirX = p.x - width * 0.5f;
                    dirY = p.y - height * 0.5f;
                    float dirLength = std::sqrt(dirX * dirX + dirY * dirY);
                    
                    if (dirLength > 0.0001f) {
                        dirX /= dirLength;
                        dirY /= dirLength;
                    } else {
                        dirX = getRandomFloat() * 2.0f - 1.0f;
                        dirY = getRandomFloat() * 2.0f - 1.0f;
                    }
                }
                break;
                
            case 2: // Spiral pattern
                {
                    float angle = getRandomFloat() * 2.0f * M_PI;
                    dirX = cos(angle + m_totalTime);
                    dirY = sin(angle + m_totalTime);
                }
                break;
                
            case 3: // Random directions
            default:
                dirX = getRandomFloat() * 2.0f - 1.0f;
                dirY = getRandomFloat() * 2.0f - 1.0f;
                break;
        }
        
        // Normalize direction
        float dirLength = std::sqrt(dirX * dirX + dirY * dirY);
        if (dirLength > 0.0001f) {
            dirX /= dirLength;
            dirY /= dirLength;
        }
        
        p.vx = dirX * velMagnitude * (0.5f + getRandomFloat() * 0.5f);
        p.vy = dirY * velMagnitude * (0.5f + getRandomFloat() * 0.5f);
        
        // Size based on energy with more variety
        if (m_beatDetected) {
            // Bigger particles during beats
            p.size = 3.0f + energy * 12.0f * getRandomFloat();
        } else {
            p.size = 2.0f + energy * 8.0f * getRandomFloat();
        }
        
        // Lifetime based on size and beat state
        if (m_beatDetected) {
            p.maxLife = 1.5f + getRandomFloat() * 1.5f; // Longer life during beats
        } else {
            p.maxLife = 1.0f + getRandomFloat() * 2.0f;
        }
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
    // Get window dimensions
    int width, height;
    m_renderEngine->getViewportSize(width, height);
    
    // Update each particle
    for (auto& p : m_particles) {
        // Update position
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
        
        // Apply gravity with music-reactive strength
        float gravityStrength = 30.0f * (1.0f + m_bassEnergy * 2.0f);
        p.vy += gravityStrength * deltaTime;
        
        // Apply drag that varies with music
        float dragFactor = 0.97f + m_trebleEnergy * 0.02f;
        p.vx *= dragFactor;
        p.vy *= dragFactor;
        
        // Bounce off edges with damping
        if (p.x < 0) {
            p.x = 0;
            p.vx = -p.vx * 0.8f;
        } else if (p.x > width) {
            p.x = width;
            p.vx = -p.vx * 0.8f;
        }
        
        if (p.y < 0) {
            p.y = 0;
            p.vy = -p.vy * 0.8f;
        } else if (p.y > height) {
            p.y = height;
            p.vy = -p.vy * 0.8f;
        }
        
        // Update lifetime with music-reactive decay
        float lifeDrain = deltaTime;
        if (m_beatDetected) {
            // Slower decay during beats for sustained visuals
            lifeDrain *= 0.7f;
        }
        p.life -= lifeDrain;
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