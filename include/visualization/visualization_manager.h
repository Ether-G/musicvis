#ifndef VISUALIZATION_MANAGER_H
#define VISUALIZATION_MANAGER_H

#include <vector>
#include <memory>
#include "visualization/visualizer.h"

class VisualizationManager {
public:
    VisualizationManager(std::shared_ptr<RenderEngine> renderEngine);
    ~VisualizationManager();

    // Initialize the visualization manager
    bool initialize();
    
    // Shutdown and cleanup
    void shutdown();
    
    // Update the current visualizer with new audio data
    void update(
        float deltaTime,
        const std::vector<float>& audioData,
        const std::vector<float>& frequencyData,
        bool beatDetected
    );
    
    // Render the current visualization
    void render();
    
    // Switch to the next visualizer
    void nextVisualizer();
    
    // Switch to a specific visualizer by index
    void setVisualizer(size_t index);
    
    // Get the name of the current visualizer
    const char* getCurrentVisualizerName() const;

private:
    // Add built-in visualizers
    void addBuiltInVisualizers();
    
    // Render engine
    std::shared_ptr<RenderEngine> m_renderEngine;
    
    // List of available visualizers
    std::vector<std::unique_ptr<Visualizer>> m_visualizers;
    
    // Index of the current visualizer
    size_t m_currentVisualizer;
};

#endif // VISUALIZATION_MANAGER_H