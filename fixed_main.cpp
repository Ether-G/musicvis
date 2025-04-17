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
