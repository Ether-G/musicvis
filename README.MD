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
graph TD
    subgraph "Initialization"
        InitMain[main.cpp starts] --> InitRE(Initialize RenderEngine);
        InitRE --> InitIH(Initialize InputHandler);
        InitIH --> InitAM(Initialize AudioManager);
        InitAM --> InitAnalysis(Initialize FFTAnalyzer & BeatDetector);
        InitAnalysis --> InitVM(Initialize VisualizationManager);
        InitVM --> LoadAudio(Load Audio File / Start Input Capture);
    end

    subgraph "Main Loop"
        LoopStart(Loop while window open) --> HandleInput(InputHandler processes events);
        HandleInput -- Space key --> VM(VisualizationManager switches visualizer);
        HandleInput -- P key --> AM(AudioManager toggles playback);
        HandleInput -- ESC key --> LoopEnd(Exit Loop);
        HandleInput --> GetAudio(AudioManager gets audio samples);
        GetAudio --> AnalyzeAudio{Audio Samples Available?};
        AnalyzeAudio -- Yes --> FFT(FFTAnalyzer processes samples);
        AnalyzeAudio -- Yes --> Beat(BeatDetector processes samples);
        FFT --> FreqData[Frequency Data];
        Beat --> BeatStatus[Beat Detected?];
        AnalyzeAudio -- No --> Render;
        FreqData --> UpdateVis(VisualizationManager updates current Visualizer);
        BeatStatus --> UpdateVis;
        GetAudio -- Audio Samples --> UpdateVis;
        UpdateVis --> Render(RenderEngine begins frame);
        Render --> VisRender(VisualizationManager renders current Visualizer);
        VisRender --> EndFrame(RenderEngine ends frame);
        EndFrame --> LoopStart;
    end

    subgraph "Components & Dependencies"
        Main(main.cpp) --> RE(RenderEngine);
        Main --> IH(InputHandler);
        Main --> AM(AudioManager);
        Main --> FFTA(FFTAnalyzer);
        Main --> BD(BeatDetector);
        Main --> VM;

        IH --> RE;
        VM --> RE;
        VM --> VBase(Visualizer : Base);
        VBase <|-- BarVis(BarVisualizer);
        VBase <|-- WaveVis(WaveVisualizer);
        VBase <|-- ParticleVis(ParticleVisualizer);
        BarVis --> RE;
        WaveVis --> RE;
        ParticleVis --> RE;

        AM --> AB(AudioBuffer);
        AM --> PortAudio[PortAudio Lib];
        AB -- libsndfile --> SndFile[libsndfile Lib];

        FFTA --> FFTW[FFTW3 Lib];

        RE --> ShaderMgr(ShaderManager);
        RE --> GLFW[GLFW Lib];
        RE --> GLEW[GLEW Lib];
        RE --> OpenGL[OpenGL Lib];
    end

    InitMain --> LoopStart;
    LoopEnd --> Shutdown(Shutdown Components);

    style InitMain fill:#f9f,stroke:#333,stroke-width:2px
    style LoopStart fill:#ccf,stroke:#333,stroke-width:2px
    style Shutdown fill:#f9f,stroke:#333,stroke-width:2px
    ```