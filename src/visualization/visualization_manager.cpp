#include <iostream>
#include "visualization/visualization_manager.h"
#include "visualization/bar_visualizer.h"
#include "visualization/wave_visualizer.h"
#include "visualization/particle_visualizer.h"
#include "render/render_engine.h"

VisualizationManager::VisualizationManager(std::shared_ptr<RenderEngine> renderEngine)
    : m_renderEngine(renderEngine)
    , m_currentVisualizer(0)
{
}

VisualizationManager::~VisualizationManager() {
    shutdown();
}

bool VisualizationManager::initialize() {
    // Add built-in visualizers
    addBuiltInVisualizers();
    
    if (m_visualizers.empty()) {
        std::cerr << "No visualizers available" << std::endl;
        return false;
    }
    
    std::cout << "Visualization manager initialized with "
              << m_visualizers.size() << " visualizers" << std::endl;
    
    // Initialize the first visualizer
    m_currentVisualizer = 0;
    if (!m_visualizers[m_currentVisualizer]->initialize()) {
        std::cerr << "Failed to initialize visualizer: "
                  << m_visualizers[m_currentVisualizer]->getName() << std::endl;
        return false;
    }
    
    std::cout << "Current visualizer: " << m_visualizers[m_currentVisualizer]->getName() << std::endl;
    
    return true;
}

void VisualizationManager::shutdown() {
    m_visualizers.clear();
}

void VisualizationManager::update(
    float deltaTime,
    const std::vector<float>& audioData,
    const std::vector<float>& frequencyData,
    bool beatDetected
) {
    if (m_currentVisualizer < m_visualizers.size()) {
        m_visualizers[m_currentVisualizer]->update(
            deltaTime,
            audioData,
            frequencyData,
            beatDetected
        );
    }
}

void VisualizationManager::render() {
    if (m_currentVisualizer < m_visualizers.size()) {
        m_visualizers[m_currentVisualizer]->render();
    }
}

void VisualizationManager::nextVisualizer() {
    if (m_visualizers.empty()) {
        return;
    }
    
    // Switch to the next visualizer
    size_t nextIndex = (m_currentVisualizer + 1) % m_visualizers.size();
    setVisualizer(nextIndex);
}

void VisualizationManager::setVisualizer(size_t index) {
    if (index >= m_visualizers.size()) {
        return;
    }
    
    if (index != m_currentVisualizer) {
        // Initialize the new visualizer if needed
        if (!m_visualizers[index]->initialize()) {
            std::cerr << "Failed to initialize visualizer: " 
                      << m_visualizers[index]->getName() << std::endl;
            return;
        }
        
        m_currentVisualizer = index;
        std::cout << "Switched to visualizer: " << m_visualizers[m_currentVisualizer]->getName() << std::endl;
    }
}

const char* VisualizationManager::getCurrentVisualizerName() const {
    if (m_currentVisualizer < m_visualizers.size()) {
        return m_visualizers[m_currentVisualizer]->getName();
    }
    return "None";
}

void VisualizationManager::addBuiltInVisualizers() {
    // Add bar visualizer
    m_visualizers.push_back(std::make_unique<BarVisualizer>(m_renderEngine));
    
    // Add wave visualizer
    m_visualizers.push_back(std::make_unique<WaveVisualizer>(m_renderEngine));
    
    // Add particle visualizer
    m_visualizers.push_back(std::make_unique<ParticleVisualizer>(m_renderEngine));
}