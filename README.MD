# Music Visualizer

A cross-platform music visualizer built with C++ using OpenGL/GLFW for rendering and PortAudio for audio processing. Inspired by classic Windows Media Player visualizations.

## Features

- Multiple visualization styles (Bar, Wave, Particle)
- Real-time FFT audio analysis
- Beat detection
- Microphone input support
- Audio file playback
- Smooth animations and transitions

## Prerequisites

- Ubuntu 22.04 (or other Linux distros with modifications)
- WSL2 (if running on Windows)
- C++17 compatible compiler
- CMake 3.10+
- GLFW3
- FFTW3
- PortAudio
- X Window System if using WSL2 (for displaying graphics)

## Setup and Installation

### 1. Install Dependencies

```bash
# Update package list
sudo apt update
sudo apt upgrade

# Install build tools
sudo apt install build-essential cmake git

# Install graphics libraries
sudo apt install libglfw3-dev libglm-dev

# Install audio libraries
sudo apt install libportaudio2 libportaudiocpp0 portaudio19-dev
sudo apt install libfftw3-dev

# Install X11 forwarding if using WSL2
sudo apt install x11-apps
```

### 2. Prepare Project

```bash
# Clone the repository
git clone https://yourrepository.com/music-visualizer.git
cd music-visualizer

# Run preparation script
chmod +x prepare_project.sh
./prepare_project.sh
```

### 3. Build

```bash
# Build the project
chmod +x build.sh
./build.sh
```

## Running the Visualizer

### From Audio File

```bash
./build/bin/music_visualizer path/to/audio-file.wav
```

### From Microphone Input

```bash
./build/bin/music_visualizer
```

## Controls

- **ESC**: Exit the application
- **SPACE**: Switch between visualizers
- **P**: Play/Pause audio (when playing from a file)

## Project Structure

```
music-visualizer/
├── src/
│   ├── audio/
│   │   ├── audio_manager.cpp
│   │   └── audio_buffer.cpp
│   ├── analysis/
│   │   ├── fft_analyzer.cpp
│   │   └── beat_detector.cpp
│   ├── visualization/
│   │   ├── visualization_manager.cpp
│   │   ├── bar_visualizer.cpp
│   │   ├── wave_visualizer.cpp
│   │   └── particle_visualizer.cpp
│   ├── render/
│   │   ├── render_engine.cpp
│   │   └── shader_manager.cpp
│   ├── input/
│   │   └── input_handler.cpp
│   └── main.cpp
├── include/
│   ├── audio/
│   ├── analysis/
│   ├── visualization/
│   ├── render/
│   └── input/
├── assets/
│   ├── shaders/
│   ├── fonts/
│   └── images/
├── build/
├── CMakeLists.txt
└── README.md
```

## Extending the Visualizer

### Adding New Visualizers

1. Create a new class that inherits from the `Visualizer` base class
2. Implement the required methods:
   - `initialize()`
   - `update()`
   - `render()`
   - `getName()`
3. Register the new visualizer in `VisualizationManager::addBuiltInVisualizers()`

## WSL2 Display Configuration

When running in WSL2, you'll need to set up X11 forwarding to display the graphics window:

1. Install an X server on Windows (like VcXsrv or Xming)
2. Start the X server
3. In WSL2, configure the DISPLAY environment variable:

```bash
export DISPLAY=:0
```

You can add this to your `~/.bashrc` file to make it permanent.

## Future Improvements

- Add more visualization types
- Support more audio file formats
- Add audio equalizer controls
- Implement shader-based effects
- Add configuration files for customization
- Support for playlist and continuous playback

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- GLFW for OpenGL context management
- PortAudio for cross-platform audio I/O
- FFTW for Fast Fourier Transform implementation