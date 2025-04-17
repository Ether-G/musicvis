#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include "render/shader_manager.h"

ShaderManager::ShaderManager()
    : m_nextId(1)
{
}

ShaderManager::~ShaderManager() {
    deleteAllShaderPrograms();
}

unsigned int ShaderManager::createShaderProgram(
    const char* vertexShaderSource,
    const char* fragmentShaderSource
) {
    // Compile vertex shader
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    if (!vertexShader) {
        return 0;
    }
    
    // Compile fragment shader
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return 0;
    }
    
    // Link shaders into a program
    unsigned int program = linkProgram(vertexShader, fragmentShader);
    
    // Delete shaders as they're linked into the program and no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (!program) {
        return 0;
    }
    
    // Store the program
    unsigned int id = m_nextId++;
    m_shaderPrograms[id] = program;
    
    return id;
}

unsigned int ShaderManager::loadShaderProgram(
    const std::string& vertexShaderPath,
    const std::string& fragmentShaderPath
) {
    // Load shader sources from files
    std::string vertexShaderSource = loadShaderSource(vertexShaderPath);
    if (vertexShaderSource.empty()) {
        std::cerr << "Failed to load vertex shader: " << vertexShaderPath << std::endl;
        return 0;
    }
    
    std::string fragmentShaderSource = loadShaderSource(fragmentShaderPath);
    if (fragmentShaderSource.empty()) {
        std::cerr << "Failed to load fragment shader: " << fragmentShaderPath << std::endl;
        return 0;
    }
    
    // Create shader program
    return createShaderProgram(
        vertexShaderSource.c_str(),
        fragmentShaderSource.c_str()
    );
}

unsigned int ShaderManager::getShaderProgram(unsigned int id) const {
    auto it = m_shaderPrograms.find(id);
    if (it != m_shaderPrograms.end()) {
        return it->second;
    }
    // If not found, log an error and return 0
    std::cerr << "Shader program ID " << id << " not found!" << std::endl;
    return 0;
}

void ShaderManager::deleteShaderProgram(unsigned int id) {
    auto it = m_shaderPrograms.find(id);
    if (it != m_shaderPrograms.end()) {
        glDeleteProgram(it->second);
        m_shaderPrograms.erase(it);
    }
}

void ShaderManager::deleteAllShaderPrograms() {
    for (const auto& pair : m_shaderPrograms) {
        glDeleteProgram(pair.second);
    }
    m_shaderPrograms.clear();
}

unsigned int ShaderManager::compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

unsigned int ShaderManager::linkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    unsigned int program = glCreateProgram();
    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader program linking error: " << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

std::string ShaderManager::loadShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}