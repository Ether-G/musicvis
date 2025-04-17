#include <iostream>
#include <GLFW/glfw3.h>
#include <chrono>
#include <GLFW/glfw3.h>
#include <thread>
#include <GLFW/glfw3.h>
#include <memory>
#include <GLFW/glfw3.h>

#include "audio/audio_manager.h"
#include <GLFW/glfw3.h>
#include "analysis/fft_analyzer.h"
#include <GLFW/glfw3.h>
#include "analysis/beat_detector.h"
#include <GLFW/glfw3.h>
#include "visualization/visualization_manager.h"
#include <GLFW/glfw3.h>
#include "render/render_engine.h"
#include <GLFW/glfw3.h>
#include "input/input_handler.h"
#include <GLFW/glfw3.h>

int main(int argc, char* argv[]) {
    try {
        std::cout << "Initializing Music Visualizer..." << std::endl;

        // Initialize rendering system
        auto renderEngine = std::make_shared<RenderEngine>();
        if (!renderEngine->initialize(1280, 720, "Music Visualizer")) {
            std::cerr << "Failed to initialize render engine" << std::endl;
            return 1;
        }

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
            }
            
            // Handle input for audio controls
            if (inputHandler->isKeyPressed(GLFW_KEY_P) && inputHandler->isKeyJustPressed()) {
                audioManager->togglePlayback();
            }

            // Get audio data
            std::vector<float> audioSamples = audioManager->getAudioSamples();
            
            // Process audio data
            if (!audioSamples.empty()) {
                // Analyze audio data
                fftAnalyzer->processAudioData(audioSamples);
                beatDetector->analyzeAudio(audioSamples);
                
                // Update visualization
                visualizationManager->update(
                    deltaTime,
                    audioSamples,
                    fftAnalyzer->getSpectrumData(),
                    beatDetector->isBeatDetected()
                );
            }

            // Render frame
            renderEngine->beginFrame();
            visualizationManager->render();
            renderEngine->endFrame();

            // Limit frame rate
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
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