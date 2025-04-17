#include <iostream>
#include <cmath>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "render/render_engine.h"
#include "render/shader_manager.h"

// Callback function for GLFW errors
static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Callback function for window resize
static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

RenderEngine::RenderEngine()
    : m_window(nullptr)
    , m_width(0)
    , m_height(0)
    , m_shaderManager(std::make_unique<ShaderManager>())
    , m_vao(0)
    , m_vbo(0)
    , m_currentShader(0)
{
}

RenderEngine::~RenderEngine() {
    shutdown();
}

bool RenderEngine::initialize(int width, int height, const std::string& title) {
    m_width = width;
    m_height = height;
    
    // Initialize GLFW
    glfwSetErrorCallback(glfwErrorCallback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);  // Enable debug context
    
    // Create window
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    // Make OpenGL context current
    glfwMakeContextCurrent(m_window);
    
    // Set resize callback
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    
    // Enable vsync
    glfwSwapInterval(1);
    
    // Initialize OpenGL with GLAD
    if (!initializeOpenGL()) {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        return false;
    }
    
    // Create shaders
    if (!createShaders()) {
        std::cerr << "Failed to create shaders" << std::endl;
        return false;
    }
    
    // Create VAO and VBO for drawing
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    // Set up VAO
    glBindVertexArray(m_vao);
    
    // Configure VBO - we need to actually allocate some initial data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // Allocate initial buffer data (empty for now, will be updated in draw calls)
    float initialData[36] = {0}; // 6 vertices * 6 components (pos + color)
    glBufferData(GL_ARRAY_BUFFER, sizeof(initialData), initialData, GL_DYNAMIC_DRAW);
    
    // Position attribute (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute (r, g, b, a)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Check for OpenGL errors after VAO/VBO setup
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after VAO/VBO setup: " << error << std::endl;
    }
    
    std::cout << "Render engine initialized: " << width << "x" << height << std::endl;
    
    return true;
}

void RenderEngine::shutdown() {
    std::cout << "Shutting down render engine..." << std::endl;
    
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    
    m_shaderManager.reset();
    
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    
    glfwTerminate();
}

bool RenderEngine::initializeOpenGL() {
    // Initialize GLAD
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    // Print OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    // Configure OpenGL
    glViewport(0, 0, m_width, m_height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return true;
}

bool RenderEngine::createShaders() {
    // Basic shader for 2D drawing
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec4 aColor;
        
        out vec4 vertexColor;
        
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * vec4(aPos, 0.0, 1.0);
            vertexColor = aColor;
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec4 vertexColor;
        
        out vec4 fragColor;
        
        void main() {
            fragColor = vertexColor;
        }
    )";
    
    unsigned int shader = m_shaderManager->createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (!shader) {
        std::cerr << "Failed to create basic shader" << std::endl;
        
        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error after shader creation: " << error << std::endl;
        }
        
        return false;
    }
    
    m_currentShader = shader;
    
    // Set projection matrix
    glUseProgram(m_shaderManager->getShaderProgram(m_currentShader));
    
    // Create orthographic projection matrix
    float left = 0.0f;
    float right = static_cast<float>(m_width);
    float bottom = static_cast<float>(m_height);
    float top = 0.0f;
    float zNear = -1.0f;
    float zFar = 1.0f;
    
    float orthoMatrix[16] = {
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -2.0f / (zFar - zNear), 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zFar + zNear) / (zFar - zNear), 1.0f
    };
    
    int projectionLoc = glGetUniformLocation(m_shaderManager->getShaderProgram(m_currentShader), "projection");
    if (projectionLoc == -1) {
        std::cerr << "Could not find projection uniform in shader" << std::endl;
    } else {
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, orthoMatrix);
    }
    
    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after setting projection matrix: " << error << std::endl;
    }
    
    std::cout << "Shaders created successfully" << std::endl;
    
    return true;
}

void RenderEngine::beginFrame() {
    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // std::cout << "Frame begun" << std::endl;
}

void RenderEngine::endFrame() {
    // Swap buffers
    glfwSwapBuffers(m_window);
    
    // Poll for events
    glfwPollEvents();
    // std::cout << "Frame ended" << std::endl;
}

bool RenderEngine::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

GLFWwindow* RenderEngine::getWindow() const {
    return m_window;
}

void RenderEngine::getViewportSize(int& width, int& height) const {
    width = m_width;
    height = m_height;
}

void RenderEngine::drawRectangle(
    float x, float y, float width, float height,
    float r, float g, float b, float a
) {
    // Vertices for a rectangle (2 triangles)
    float vertices[] = {
        // Positions (x, y)        // Colors (r, g, b, a)
        x, y,                      r, g, b, a,
        x + width, y,              r, g, b, a,
        x + width, y + height,     r, g, b, a,
        
        x, y,                      r, g, b, a,
        x + width, y + height,     r, g, b, a,
        x, y + height,             r, g, b, a
    };
    
    // Bind shader
    glUseProgram(m_shaderManager->getShaderProgram(m_currentShader));
    
    // Bind VAO
    glBindVertexArray(m_vao);
    
    // Update VBO data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    // Make sure attributes are enabled
    glEnableVertexAttribArray(0); // Position
    glEnableVertexAttribArray(1); // Color
    
    // Check for errors after buffer data
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after buffer data: " << error << std::endl;
    }
    
    // Draw
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Check for errors after draw
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after draw: " << error << std::endl;
    }
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderEngine::drawCircle(
    float x, float y, float radius, int segments,
    float r, float g, float b, float a
) {
    // Ensure minimum number of segments
    segments = std::max(8, segments);
    
    // Calculate vertices for a circle
    std::vector<float> vertices;
    vertices.reserve((segments + 2) * 6); // 6 floats per vertex
    
    // Center vertex
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(r);
    vertices.push_back(g);
    vertices.push_back(b);
    vertices.push_back(a);
    
    // Outer vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float vx = x + radius * cos(angle);
        float vy = y + radius * sin(angle);
        
        vertices.push_back(vx);
        vertices.push_back(vy);
        vertices.push_back(r);
        vertices.push_back(g);
        vertices.push_back(b);
        vertices.push_back(a);
    }
    
    // Bind shader
    glUseProgram(m_currentShader);
    
    // Bind VAO
    glBindVertexArray(m_vao);
    
    // Update VBO data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    
    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderEngine::drawLine(
    float x1, float y1, float x2, float y2, float thickness,
    float r, float g, float b, float a
) {
    // Calculate perpendicular direction
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length < 0.0001f) {
        return; // Line too short
    }
    
    // Normalize
    dx /= length;
    dy /= length;
    
    // Perpendicular vector
    float px = -dy * (thickness * 0.5f);
    float py = dx * (thickness * 0.5f);
    
    // Calculate the four corners of the line segment
    float vertices[] = {
        // Positions (x, y)            // Colors (r, g, b, a)
        x1 + px, y1 + py,              r, g, b, a,
        x2 + px, y2 + py,              r, g, b, a,
        x2 - px, y2 - py,              r, g, b, a,
        
        x1 + px, y1 + py,              r, g, b, a,
        x2 - px, y2 - py,              r, g, b, a,
        x1 - px, y1 - py,              r, g, b, a
    };
    
    // Bind shader
    glUseProgram(m_currentShader);
    
    // Bind VAO
    glBindVertexArray(m_vao);
    
    // Update VBO data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    // Draw
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderEngine::drawLines(
    const float* points, int count, float thickness,
    float r, float g, float b, float a
) {
    if (count < 2) {
        return; // Need at least 2 points for a line
    }
    
    // Draw each line segment
    for (int i = 0; i < count - 1; ++i) {
        drawLine(
            points[i * 2], points[i * 2 + 1],
            points[(i + 1) * 2], points[(i + 1) * 2 + 1],
            thickness, r, g, b, a
        );
    }
}

void RenderEngine::drawPoints(
    const float* points, int count, float size,
    float r, float g, float b, float a
) {
    // Draw each point as a small circle
    for (int i = 0; i < count; ++i) {
        drawCircle(
            points[i * 2], points[i * 2 + 1],
            size * 0.5f, 8,
            r, g, b, a
        );
    }
}