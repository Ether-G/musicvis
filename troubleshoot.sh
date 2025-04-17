#!/bin/bash

echo "Running advanced troubleshooting..."

# Check OpenGL development libraries
echo "Checking OpenGL development libraries:"
if dpkg -l | grep -q libgl1-mesa-dev; then
    echo "[OK] OpenGL development libraries installed"
else
    echo "[ERROR] OpenGL development libraries not installed"
    echo "       Run: sudo apt install libgl1-mesa-dev"
fi

# Check GLEW development libraries
echo "Checking GLEW development libraries:"
if dpkg -l | grep -q libglew-dev; then
    echo "[OK] GLEW development libraries installed"
else
    echo "[ERROR] GLEW development libraries not installed"
    echo "       Run: sudo apt install libglew-dev"
fi

# Check GLFW development libraries
echo "Checking GLFW development libraries:"
if dpkg -l | grep -q libglfw3-dev; then
    echo "[OK] GLFW development libraries installed"
else
    echo "[ERROR] GLFW development libraries not installed"
    echo "       Run: sudo apt install libglfw3-dev"
fi

# Check FFTW development libraries
echo "Checking FFTW development libraries:"
if dpkg -l | grep -q libfftw3-dev; then
    echo "[OK] FFTW development libraries installed"
else
    echo "[ERROR] FFTW development libraries not installed"
    echo "       Run: sudo apt install libfftw3-dev"
fi

# Check PortAudio development libraries
echo "Checking PortAudio development libraries:"
if dpkg -l | grep -q portaudio19-dev; then
    echo "[OK] PortAudio development libraries installed"
else
    echo "[ERROR] PortAudio development libraries not installed"
    echo "       Run: sudo apt install portaudio19-dev"
fi

# Check if there are any issues with OpenGL rendering
echo "Checking if there are any issues with OpenGL rendering:"
if glxinfo | grep -q "direct rendering: Yes"; then
    echo "[OK] Direct rendering is enabled"
else
    echo "[WARNING] Direct rendering might not be enabled"
    echo "         This could cause rendering issues"
fi

# Check if X11 forwarding is working (if using WSL)
echo "Checking X11 forwarding (if using WSL):"
if echo :1 | grep -q ":"; then
    echo "[OK] DISPLAY variable is set to: :1"
else
    echo "[WARNING] DISPLAY variable is not set correctly"
    echo "         This will cause issues if you're using WSL"
    echo "         Make sure you have an X server running on Windows"
    echo "         and export DISPLAY=:0 (or appropriate value)"
fi

echo ""
echo "Creating simple test program to verify OpenGL works:"
cat > test_opengl.cpp << 'EOT'
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Test", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // Make the window's context current
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }
    
    // Print OpenGL version
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // Main loop
    std::cout << "Starting rendering loop..." << std::endl;
    while (!glfwWindowShouldClose(window)) {
        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Swap front and back buffers
        glfwSwapBuffers(window);
        
        // Poll for and process events
        glfwPollEvents();
    }
    
    // Clean up
    glfwTerminate();
    std::cout << "Test completed successfully" << std::endl;
    return 0;
}
EOT

echo "Compiling test program:"
g++ -o test_opengl test_opengl.cpp -lGL -lGLEW -lglfw

if [ -f test_opengl ]; then
    echo "[OK] Test program compiled successfully"
    echo "Run it with: ./test_opengl"
else
    echo "[ERROR] Failed to compile test program"
fi

echo ""
echo "Troubleshooting completed. Use the information above to fix any issues."
echo "If you're using WSL, make sure you have an X server running on Windows."
echo "You can use VcXsrv, Xming, or another X server for Windows."
