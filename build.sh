#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Run CMake
cmake ..

# Build the project
make -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful! Executable is located at: build/bin/music_visualizer"
    echo "Run with: ./bin/music_visualizer [audio_file.wav]"
else
    echo "Build failed!"
fi