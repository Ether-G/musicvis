#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <string>
#include <unordered_map>

class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();

    // Create a shader program from vertex and fragment shader sources
    unsigned int createShaderProgram(
        const char* vertexShaderSource,
        const char* fragmentShaderSource
    );
    
    // Load a shader program from files
    unsigned int loadShaderProgram(
        const std::string& vertexShaderPath,
        const std::string& fragmentShaderPath
    );
    
    // Get a shader program by ID
    unsigned int getShaderProgram(unsigned int id) const;
    
    // Delete a shader program
    void deleteShaderProgram(unsigned int id);
    
    // Delete all shader programs
    void deleteAllShaderPrograms();

private:
    // Compile a shader from source
    unsigned int compileShader(unsigned int type, const char* source);
    
    // Link shaders into a program
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    
    // Load shader source from file
    std::string loadShaderSource(const std::string& filePath);
    
    // Map of shader program IDs to program handles
    std::unordered_map<unsigned int, unsigned int> m_shaderPrograms;
    
    // Next available shader program ID
    unsigned int m_nextId;
};

#endif // SHADER_MANAGER_H