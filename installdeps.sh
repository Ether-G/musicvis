#!/bin/bash

echo "Installing required dependencies for Music Visualizer..."

# Update package list
sudo apt update

# Install build tools
sudo apt install -y build-essential cmake git

# Install graphics libraries
sudo apt install -y libglfw3-dev libglew-dev libglm-dev

# Install audio libraries
sudo apt install -y libportaudio2 libportaudiocpp0 portaudio19-dev
sudo apt install -y libfftw3-dev

# Install X11 forwarding if using WSL2
sudo apt install -y x11-apps

echo "All dependencies installed successfully!"
echo "Now run the fix_includes.sh script to fix include issues."