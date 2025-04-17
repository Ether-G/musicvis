#!/bin/bash

echo "========================================"
echo "Music Visualizer Debugging Script"
echo "========================================"

# 1. First, let's check if the render_engine.cpp was properly fixed
if grep -q "gladLoadGLLoader" src/render/render_engine.cpp; then
    echo "[ERROR] Your render_engine.cpp still contains GLAD code"
    echo "       Run the fixerr.sh script to fix this"
    echo ""
else
    echo "[OK] render_engine.cpp appears to use GLEW properly"
fi

# 2. Let's add some debug prints to the code
echo ""
echo "Adding debug prints to the code..."

# Add debug in main.cpp
cat > debug_main.patch << EOF
--- main.cpp    2023-01-01 00:00:00.000000000 +0000
+++ main.cpp    2023-01-01 00:00:00.000000000 +0000
@@ -85,6 +85,9 @@
             // Process audio data
             if (!audioSamples.empty()) {
                 // Analyze audio data
+                std::cout << "Audio samples received: " << audioSamples.size() << std::endl;
+                std::cout << "Processing audio data..." << std::endl;
+                
                 fftAnalyzer->processAudioData(audioSamples);
                 beatDetector->analyzeAudio(audioSamples);
                 
@@ -97,7 +100,15 @@
 
             // Render frame
             renderEngine->beginFrame();
+            
+            // DEBUG: Draw a test rectangle to verify rendering works
+            std::cout << "Drawing test rectangle..." << std::endl;
+            renderEngine->drawRectangle(
+                width * 0.25f, height * 0.25f, width * 0.5f, height * 0.5f,
+                1.0f, 0.0f, 0.0f, 1.0f
+            );
             visualizationManager->render();
+            std::cout << "Frame rendered" << std::endl;
             renderEngine->endFrame();
 
             // Limit frame rate
EOF

# Add debug in render_engine.cpp
cat > debug_render.patch << EOF
--- src/render/render_engine.cpp    2023-01-01 00:00:00.000000000 +0000
+++ src/render/render_engine.cpp    2023-01-01 00:00:00.000000000 +0000
@@ -50,6 +50,7 @@
     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
+    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);  // Enable debug context
     
     // Create window
     m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
@@ -82,6 +83,14 @@
     // Create VAO and VBO for drawing
     glGenVertexArrays(1, &m_vao);
     glGenBuffers(1, &m_vbo);
+
+    // Check for OpenGL errors
+    GLenum error = glGetError();
+    if (error != GL_NO_ERROR) {
+        std::cerr << "OpenGL error during initialization: " << error << std::endl;
+    } else {
+        std::cout << "OpenGL initialization successful" << std::endl;
+    }
     
     // Set up VAO
     glBindVertexArray(m_vao);
@@ -103,6 +112,8 @@
 }
 
 void RenderEngine::shutdown() {
+    std::cout << "Shutting down render engine..." << std::endl;
+    
     if (m_vao) {
         glDeleteVertexArrays(1, &m_vao);
         m_vao = 0;
@@ -120,18 +131,28 @@
     }
     
     glfwTerminate();
+    std::cout << "Render engine shutdown complete" << std::endl;
 }
 
 bool RenderEngine::initializeOpenGL() {
     // Initialize GLEW
-    if (glewInit() != GLEW_OK) {
-        std::cerr << "Failed to initialize GLEW" << std::endl;
+    GLenum err = glewInit();
+    if (err != GLEW_OK) {
+        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
         return false;
     }
     
     // Print OpenGL version
-    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
-    
+    const GLubyte* version = glGetString(GL_VERSION);
+    const GLubyte* vendor = glGetString(GL_VENDOR);
+    const GLubyte* renderer = glGetString(GL_RENDERER);
+    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
+
+    std::cout << "OpenGL Version: " << version << std::endl;
+    std::cout << "Vendor: " << vendor << std::endl;
+    std::cout << "Renderer: " << renderer << std::endl;
+    std::cout << "GLSL Version: " << glslVersion << std::endl;
+
     // Configure OpenGL
     glViewport(0, 0, m_width, m_height);
     glEnable(GL_BLEND);
@@ -144,6 +165,8 @@
     const char* vertexShaderSource = R"(
         #version 330 core
         layout (location = 0) in vec2 aPos;
+        // Add debug code
+        // DEBUG: Input attributes
         layout (location = 1) in vec4 aColor;
         
         out vec4 vertexColor;
@@ -158,6 +181,7 @@
     
     const char* fragmentShaderSource = R"(
         #version 330 core
+        // DEBUG: Fragment shader input
         in vec4 vertexColor;
         
         out vec4 fragColor;
@@ -170,6 +194,14 @@
     unsigned int shader = m_shaderManager->createShaderProgram(vertexShaderSource, fragmentShaderSource);
     if (!shader) {
         std::cerr << "Failed to create basic shader" << std::endl;
+        
+        // Check for OpenGL errors
+        GLenum error = glGetError();
+        if (error != GL_NO_ERROR) {
+            std::cerr << "OpenGL error after shader creation: " << error << std::endl;
+        }
+        
+        std::cerr << "Shader creation failed!" << std::endl;
         return false;
     }
     
@@ -192,6 +224,8 @@
     
     int projectionLoc = glGetUniformLocation(m_currentShader, "projection");
     glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, orthoMatrix);
+
+    std::cout << "Shaders created successfully" << std::endl;
     
     return true;
 }
@@ -200,6 +234,7 @@
     // Clear the screen
     glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
     glClear(GL_COLOR_BUFFER_BIT);
+    // std::cout << "Frame begun" << std::endl;
 }
 
 void RenderEngine::endFrame() {
@@ -208,6 +243,7 @@
     
     // Poll for events
     glfwPollEvents();
+    // std::cout << "Frame ended" << std::endl;
 }
 
 bool RenderEngine::shouldClose() const {
@@ -243,6 +279,8 @@
         x, y + height,             r, g, b, a
     };
     
+    // std::cout << "Drawing rectangle at (" << x << ", " << y << ") with size " << width << "x" << height << std::endl;
+    
     // Bind shader
     glUseProgram(m_currentShader);
     
@@ -257,6 +295,13 @@
     
     // Unbind
     glBindBuffer(GL_ARRAY_BUFFER, 0);
+    
+    // Check for OpenGL errors
+    GLenum error = glGetError();
+    if (error != GL_NO_ERROR) {
+        std::cerr << "OpenGL error after drawing rectangle: " << error << std::endl;
+    }
+    
     glBindVertexArray(0);
 }
EOF

# Add debug in visualization_manager.cpp
cat > debug_vizmanager.patch << EOF
--- src/visualization/visualization_manager.cpp    2023-01-01 00:00:00.000000000 +0000
+++ src/visualization/visualization_manager.cpp    2023-01-01 00:00:00.000000000 +0000
@@ -51,8 +51,14 @@
 ) {
     if (m_currentVisualizer < m_visualizers.size()) {
+        std::cout << "Updating visualizer: " << m_visualizers[m_currentVisualizer]->getName() << std::endl;
+        std::cout << "  - Audio data size: " << audioData.size() << std::endl;
+        std::cout << "  - Frequency data size: " << frequencyData.size() << std::endl;
+        std::cout << "  - Beat detected: " << (beatDetected ? "Yes" : "No") << std::endl;
+        
         m_visualizers[m_currentVisualizer]->update(
             deltaTime,
             audioData,
             frequencyData,
             beatDetected
         );
+        std::cout << "Visualizer updated" << std::endl;
     }
 }
 
 void VisualizationManager::render() {
     if (m_currentVisualizer < m_visualizers.size()) {
+        std::cout << "Rendering visualizer: " << m_visualizers[m_currentVisualizer]->getName() << std::endl;
         m_visualizers[m_currentVisualizer]->render();
+        std::cout << "Visualizer rendered" << std::endl;
     }
 }
EOF

# Try to apply patches
patch main.cpp debug_main.patch
patch src/render/render_engine.cpp debug_render.patch
patch src/visualization/visualization_manager.cpp debug_vizmanager.patch

echo ""
echo "3. Checking if CMakeLists.txt includes all necessary libraries"

if ! grep -q "fftw3" CMakeLists.txt; then
    echo "[WARNING] FFTW3 might not be properly linked in CMakeLists.txt"
else
    echo "[OK] FFTW3 is linked in CMakeLists.txt"
fi

if ! grep -q "glfw" CMakeLists.txt; then
    echo "[WARNING] GLFW might not be properly linked in CMakeLists.txt"
else
    echo "[OK] GLFW is linked in CMakeLists.txt"
fi

if ! grep -q "GLEW" CMakeLists.txt; then
    echo "[WARNING] GLEW might not be properly linked in CMakeLists.txt"
else
    echo "[OK] GLEW is linked in CMakeLists.txt"
fi

if ! grep -q "portaudio" CMakeLists.txt; then
    echo "[WARNING] PortAudio might not be properly linked in CMakeLists.txt"
else
    echo "[OK] PortAudio is linked in CMakeLists.txt"
fi

echo ""
echo "4. Creating a fixed_main.cpp with test visualization"
cat > fixed_main.cpp << EOF
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>
#include <memory>

#include "audio/audio_manager.h"
#include "analysis/fft_analyzer.h"
#include "analysis/beat_detector.h"
#include "visualization/visualization_manager.h"
#include "render/render_engine.h"
#include "input/input_handler.h"

int main(int argc, char* argv[]) {
    try {
        std::cout << "Initializing Music Visualizer..." << std::endl;

        // Initialize rendering system
        auto renderEngine = std::make_shared<RenderEngine>();
        if (!renderEngine->initialize(1280, 720, "Music Visualizer")) {
            std::cerr << "Failed to initialize render engine" << std::endl;
            return 1;
        }

        // Get viewport size
        int width, height;
        renderEngine->getViewportSize(width, height);

        // Initialize input handler
        auto inputHandler = std::make_shared<InputHandler>(renderEngine->getWindow());
        if (!inputHandler->initialize()) {
            std::cerr << "Failed to initialize input handler" << std::endl;
            return 1;
        }

        // Initialize audio system
        auto audioManager = std::make_shared<AudioManager>();
        if (!audioManager->initialize()) {
            std::cerr << "Failed to initialize audio system" << std::endl;
            return 1;
        }

        // Initialize audio analysis
        auto fftAnalyzer = std::make_shared<FFTAnalyzer>();
        if (!fftAnalyzer->initialize(2048)) { // 2048 sample window size
            std::cerr << "Failed to initialize FFT analyzer" << std::endl;
            return 1;
        }

        auto beatDetector = std::make_shared<BeatDetector>();
        if (!beatDetector->initialize(0.15f)) { // 0.15 sensitivity
            std::cerr << "Failed to initialize beat detector" << std::endl;
            return 1;
        }

        // Initialize visualization system
        auto visualizationManager = std::make_shared<VisualizationManager>(renderEngine);
        if (!visualizationManager->initialize()) {
            std::cerr << "Failed to initialize visualization manager" << std::endl;
            return 1;
        }

        // Load audio if specified in command arguments
        if (argc > 1) {
            if (!audioManager->loadFile(argv[1])) {
                std::cerr << "Failed to load audio file: " << argv[1] << std::endl;
                return 1;
            }
            audioManager->play();
        } else {
            if (!audioManager->startInputCapture()) {
                std::cerr << "Failed to start audio input capture" << std::endl;
                return 1;
            }
        }

        std::cout << "Music Visualizer initialized successfully" << std::endl;
        std::cout << "Press ESC to exit, SPACE to switch visualizer" << std::endl;

        // Main loop
        auto lastTime = std::chrono::high_resolution_clock::now();
        int frameCount = 0;
        
        // Test colors for visualization
        float colors[4][4] = {
            {1.0f, 0.0f, 0.0f, 1.0f}, // Red
            {0.0f, 1.0f, 0.0f, 1.0f}, // Green
            {0.0f, 0.0f, 1.0f, 1.0f}, // Blue
            {1.0f, 1.0f, 0.0f, 1.0f}  // Yellow
        };
        int currentColor = 0;
        
        while (!renderEngine->shouldClose()) {
            // Calculate delta time
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Process input
            inputHandler->update();
            
            // Handle input for switching visualizers
            if (inputHandler->isKeyPressed(GLFW_KEY_SPACE) && inputHandler->isKeyJustPressed()) {
                visualizationManager->nextVisualizer();
                std::cout << "Switched to visualizer: " << visualizationManager->getCurrentVisualizerName() << std::endl;
            }
            
            // Handle input for audio controls
            if (inputHandler->isKeyPressed(GLFW_KEY_P) && inputHandler->isKeyJustPressed()) {
                audioManager->togglePlayback();
                std::cout << "Toggled audio playback" << std::endl;
            }
            
            // Handle color switching for test rectangle
            if (inputHandler->isKeyPressed(GLFW_KEY_C) && inputHandler->isKeyJustPressed()) {
                currentColor = (currentColor + 1) % 4;
                std::cout << "Switched test rectangle color" << std::endl;
            }

            // Get audio data
            std::vector<float> audioSamples = audioManager->getAudioSamples();
            
            // Process audio data
            if (!audioSamples.empty()) {
                // Print some debug about audio
                if (frameCount % 60 == 0) { // Only print once per second at 60fps
                    std::cout << "Audio sample count: " << audioSamples.size() << std::endl;
                    
                    // Print a few sample values
                    if (audioSamples.size() >= 4) {
                        std::cout << "Sample values: " 
                                  << audioSamples[0] << ", " 
                                  << audioSamples[1] << ", "
                                  << audioSamples[2] << ", "
                                  << audioSamples[3] << std::endl;
                    }
                }
                
                // Analyze audio data
                fftAnalyzer->processAudioData(audioSamples);
                std::vector<float> spectrum = fftAnalyzer->getSpectrumData();
                
                if (frameCount % 60 == 0 && !spectrum.empty()) {
                    std::cout << "Spectrum data count: " << spectrum.size() << std::endl;
                    
                    // Print a few spectrum values
                    if (spectrum.size() >= 4) {
                        std::cout << "Spectrum values: " 
                                  << spectrum[0] << ", " 
                                  << spectrum[1] << ", "
                                  << spectrum[2] << ", "
                                  << spectrum[3] << std::endl;
                    }
                }
                
                beatDetector->analyzeAudio(audioSamples);
                bool beat = beatDetector->isBeatDetected();
                
                if (beat && frameCount % 10 == 0) {
                    std::cout << "Beat detected!" << std::endl;
                }
                
                // Update visualization
                visualizationManager->update(
                    deltaTime,
                    audioSamples,
                    spectrum,
                    beat
                );
            }

            // Render frame
            renderEngine->beginFrame();
            
            // Draw a test rectangle that changes color
            renderEngine->drawRectangle(
                width * 0.25f, height * 0.25f, 
                width * 0.5f, height * 0.5f,
                colors[currentColor][0], 
                colors[currentColor][1], 
                colors[currentColor][2], 
                colors[currentColor][3]
            );
            
            // Draw the visualizer
            visualizationManager->render();
            
            renderEngine->endFrame();

            // Limit frame rate
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 fps
            
            frameCount++;
        }

        // Cleanup
        audioManager->shutdown();
        visualizationManager->shutdown();
        renderEngine->shutdown();

        std::cout << "Music Visualizer shut down successfully" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception caught" << std::endl;
        return 1;
    }
}
EOF

echo ""
echo "5. Creating additional troubleshooting script"
cat > troubleshoot.sh << EOF
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
if echo $DISPLAY | grep -q ":"; then
    echo "[OK] DISPLAY variable is set to: $DISPLAY"
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
EOF

chmod +x troubleshoot.sh

echo ""
echo "========================================"
echo "Debugging setup complete!"
echo "========================================"
echo ""
echo "Next steps:"
echo "1. Run the build script to rebuild your project:"
echo "   $ ./build.sh"
echo ""
echo "2. If the build fails, run the troubleshooting script:"
echo "   $ ./troubleshoot.sh"
echo ""
echo "3. If the visualizers still don't work after building,"
echo "   try the fixed main.cpp:"
echo "   $ cp fixed_main.cpp src/main.cpp"
echo "   $ ./build.sh"
echo ""
echo "4. Look for error messages in the console output"
echo "   to identify specific issues."
echo ""
echo "Good luck debugging your Music Visualizer!"