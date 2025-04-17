#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <string>
#include <memory>

// Forward declarations
struct GLFWwindow;
class ShaderManager;

class RenderEngine {
public:
    RenderEngine();
    ~RenderEngine();

    // Initialize the rendering engine
    bool initialize(int width, int height, const std::string& title);
    
    // Shutdown and cleanup
    void shutdown();
    
    // Begin frame rendering
    void beginFrame();
    
    // End frame rendering
    void endFrame();
    
    // Check if the window should close
    bool shouldClose() const;
    
    // Get the window handle
    GLFWwindow* getWindow() const;
    
    // Get the viewport size
    void getViewportSize(int& width, int& height) const;
    
    // Drawing primitives
    void drawRectangle(float x, float y, float width, float height, 
                       float r, float g, float b, float a);
    
    void drawCircle(float x, float y, float radius, int segments, 
                    float r, float g, float b, float a);
    
    void drawLine(float x1, float y1, float x2, float y2, float thickness, 
                  float r, float g, float b, float a);
    
    void drawLines(const float* points, int count, float thickness, 
                   float r, float g, float b, float a);
    
    void drawPoints(const float* points, int count, float size, 
                    float r, float g, float b, float a);

private:
    // Initialize OpenGL
    bool initializeOpenGL();
    
    // Create shaders
    bool createShaders();
    
    // GLFW window
    GLFWwindow* m_window;
    
    // Window dimensions
    int m_width;
    int m_height;
    
    // Shader manager
    std::unique_ptr<ShaderManager> m_shaderManager;
    
    // Vertex Array Object for drawing
    unsigned int m_vao;
    
    // Vertex Buffer Object for drawing
    unsigned int m_vbo;
    
    // Current shader program
    unsigned int m_currentShader;
};

#endif // RENDER_ENGINE_H