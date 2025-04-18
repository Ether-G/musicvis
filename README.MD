# Music Visualizer

**Version:** 0.1

A C++ application that visualizes audio input in real-time using various graphical effects. It can process audio from either a file or live microphone input.

## Features

* **Real-time Audio Visualization:** Renders graphics synchronized with audio input.
* **Multiple Visualizers:** Includes several visualization modes:
    * Bar Visualizer
    * Wave Visualizer
    * Particle Visualizer
* **Audio Input Options:**
    * Load and play audio files (requires libsndfile).
    * Capture live audio from microphone/input device.
* **Audio Analysis:**
    * Fast Fourier Transform (FFT) for frequency spectrum data.
    * Beat detection algorithm.
* **OpenGL Rendering:** Uses OpenGL for rendering graphics.
* **Cross-Platform Libraries:** Utilizes libraries like GLFW, GLEW, PortAudio, and FFTW3.

## Compatibility

* **Operating System:** Currently developed and tested primarily on **Ubuntu Linux**.
* **Windows Subsystem for Linux (WSL):** Known issues exist when running directly in WSL. Graphics and audio may not function correctly. Native Ubuntu is recommended.

## Dependencies

The following libraries are required to build and run the project:

* **Build Tools:**
    * CMake (version 3.10 or higher) [cite: 53]
    * A C++17 compliant compiler (like GCC/g++) [cite: 53]
    * Make
    * Git (optional, for cloning)
* **Graphics:**
    * OpenGL [cite: 53]
    * GLEW [cite: 53]
    * GLFW3 [cite: 53]
    * GLM (Note: Included via `installdeps.sh` but not explicitly in CMakeLists.txt find_package)
* **Audio:**
    * PortAudio (for audio I/O)
    * FFTW3 (for FFT analysis)
    * libsndfile (optional, for loading audio files)

## Building

1.  **Clone the Repository (if you haven't already):**
    ```bash
    git clone <your-repository-url>
    cd musicvis
    ```

2.  **Install Dependencies (Ubuntu):**
    Run the provided script to install necessary libraries:
    ```bash
    chmod +x installdeps.sh
    ./installdeps.sh
    ```
    *Note: This script also installs `x11-apps`, which might be needed for GUI display, especially in WSL, though WSL usage is not fully supported.*

3.  **Run CMake and Build:**
    Use the build script:
    ```bash
    chmod +x build.sh
    ./build.sh
    ```
    Alternatively, build manually:
    ```bash
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    ```
    The executable will be created in the `build/bin/` directory.

## Usage

Run the visualizer from the `build` directory:

* **Live Audio Input:**
    ```bash
    ./bin/music_visualizer
    ```
    *(This will attempt to use the default audio input device).*

* **Audio File Input:**
    ```bash
    ./bin/music_visualizer /path/to/your/audio_file.wav
    ```
    *(Requires libsndfile to be installed and detected during build).*

**Controls:**

* `SPACE`: Switch to the next visualizer.
* `P`: Toggle play/pause for audio file playback.
* `ESC`: Exit the application.

## Architecture

```mermaid
classDiagram
    class Main {
        +main(argc, argv)
    }
    class RenderEngine {
        +initialize(width, height, title) bool
        +shutdown()
        +beginFrame()
        +endFrame()
        +shouldClose() bool
        +getWindow() GLFWwindow*
        +getViewportSize(width, height)
        +drawRectangle()
        +drawCircle()
        +drawLine()
        +drawLines()
        +drawPoints()
        -m_window: GLFWwindow*
        -m_shaderManager: unique_ptr~ShaderManager~
        -m_vao: unsigned int
        -m_vbo: unsigned int
    }
    class ShaderManager {
        +createShaderProgram() unsigned int
        +loadShaderProgram() unsigned int
        +getShaderProgram(id) unsigned int
        -m_shaderPrograms: unordered_map~int, int~
    }
    class InputHandler {
        +initialize() bool
        +update()
        +isKeyPressed(key) bool
        +isKeyJustPressed() bool
        +getMousePosition(x, y)
        -m_window: GLFWwindow*
        -m_keyStates
        -m_mouseButtonStates
    }
    class AudioManager {
        +initialize() bool
        +shutdown()
        +loadFile(filePath) bool
        +startInputCapture() bool
        +play() bool
        +pause() bool
        +togglePlayback()
        +getAudioSamples() vector~float~
        -m_stream: PaStream*
        -m_audioBuffer: shared_ptr~AudioBuffer~
        -m_currentSamples: vector~float~
        -m_mutex: mutex
    }
    class AudioBuffer {
        +loadFromFile(filePath) bool
        +getSamples(numSamples) vector~float~
        +reset()
        -m_audioData: vector~float~
        -m_position: size_t
        -m_mutex: mutex
    }
    class FFTAnalyzer {
        +initialize(windowSize) bool
        +processAudioData(audioData)
        +getSpectrumData() vector~float~
        -m_fftInput: double*
        -m_fftOutput: fftw_complex*
        -m_fftPlan: fftw_plan
        -m_magnitudes: vector~float~
    }
    class BeatDetector {
        +initialize(sensitivity) bool
        +analyzeAudio(audioData)
        +isBeatDetected() bool
        -m_energyHistory: deque~float~
        -m_threshold: float
        -m_beatDetected: bool
    }
    class VisualizationManager {
        +initialize() bool
        +update(deltaTime, audioData, freqData, beatDetected)
        +render()
        +nextVisualizer()
        -m_renderEngine: shared_ptr~RenderEngine~
        -m_visualizers: vector~unique_ptr~Visualizer~~
        -m_currentVisualizer: size_t
    }
    class Visualizer {
        <<Abstract>>
        +initialize() bool
        +update(deltaTime, audioData, freqData, beatDetected)
        +render()
        +getName() const char*
        #m_renderEngine: shared_ptr~RenderEngine~
    }
    class BarVisualizer {
        +initialize() bool
        +update(deltaTime, audioData, freqData, beatDetected)
        +render()
        +getName() const char*
    }
    class WaveVisualizer {
        +initialize() bool
        +update(deltaTime, audioData, freqData, beatDetected)
        +render()
        +getName() const char*
    }
    class ParticleVisualizer {
        +initialize() bool
        +update(deltaTime, audioData, freqData, beatDetected)
        +render()
        +getName() const char*
    }

    Main --> RenderEngine : uses
    Main --> InputHandler : uses
    Main --> AudioManager : uses
    Main --> FFTAnalyzer : uses
    Main --> BeatDetector : uses
    Main --> VisualizationManager : uses

    RenderEngine --> ShaderManager : uses

    AudioManager --> AudioBuffer : uses

    VisualizationManager --> RenderEngine : uses
    VisualizationManager o-- Visualizer : manages >

    Visualizer <|-- BarVisualizer
    Visualizer <|-- WaveVisualizer
    Visualizer <|-- ParticleVisualizer

    BarVisualizer --> RenderEngine : uses
    WaveVisualizer --> RenderEngine : uses
    ParticleVisualizer --> RenderEngine : uses
    ```