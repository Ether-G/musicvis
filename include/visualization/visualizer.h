#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <vector>
#include <memory>

// Forward declarations
class RenderEngine;

// Abstract base class for visualizers
class Visualizer {
public:
    Visualizer(std::shared_ptr<RenderEngine> renderEngine);
    virtual ~Visualizer();

    // Initialize the visualizer
    virtual bool initialize() = 0;
    
    // Update visualizer state with new audio data
    virtual void update(
        float deltaTime,
        const std::vector<float>& audioData,
        const std::vector<float>& frequencyData,
        bool beatDetected
    ) = 0;
    
    // Render the visualization
    virtual void render() = 0;
    
    // Get the visualizer name
    virtual const char* getName() const = 0;

protected:
    // Shared render engine
    std::shared_ptr<RenderEngine> m_renderEngine;
};

#endif // VISUALIZER_H