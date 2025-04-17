#!/bin/bash

# Script to verify all source files are in the correct locations

echo "Verifying project file structure..."
echo "====================================="
echo ""

# Directory structure verification
echo "Directory Structure:"
echo "-------------------"
for dir in \
    "src" "src/audio" "src/analysis" "src/visualization" "src/render" "src/input" \
    "include" "include/audio" "include/analysis" "include/visualization" "include/render" "include/input" \
    "assets" "assets/shaders" "assets/fonts" "assets/images"
do
    if [ -d "$dir" ]; then
        echo "✓ $dir (exists)"
    else
        echo "✗ $dir (missing)"
    fi
done

echo ""
echo "Source Files (.cpp):"
echo "-------------------"

# Expected source files
expected_cpp_files=(
    "src/main.cpp"
    "src/audio/audio_manager.cpp"
    "src/audio/audio_buffer.cpp"
    "src/analysis/fft_analyzer.cpp"
    "src/analysis/beat_detector.cpp"
    "src/visualization/visualization_manager.cpp"
    "src/visualization/visualizer.cpp"
    "src/visualization/bar_visualizer.cpp"
    "src/visualization/wave_visualizer.cpp"
    "src/visualization/particle_visualizer.cpp"
    "src/render/render_engine.cpp"
    "src/render/shader_manager.cpp"
    "src/input/input_handler.cpp"
)

# Check each expected source file
for file in "${expected_cpp_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file (exists, $(wc -l < "$file") lines)"
    else
        echo "✗ $file (missing)"
    fi
done

echo ""
echo "Header Files (.h):"
echo "-------------------"

# Expected header files
expected_h_files=(
    "include/audio/audio_manager.h"
    "include/audio/audio_buffer.h"
    "include/analysis/fft_analyzer.h"
    "include/analysis/beat_detector.h"
    "include/visualization/visualizer.h"
    "include/visualization/visualization_manager.h"
    "include/visualization/bar_visualizer.h"
    "include/visualization/wave_visualizer.h"
    "include/visualization/particle_visualizer.h"
    "include/render/render_engine.h"
    "include/render/shader_manager.h"
    "include/input/input_handler.h"
)

# Check each expected header file
for file in "${expected_h_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file (exists, $(wc -l < "$file") lines)"
    else
        echo "✗ $file (missing)"
    fi
done

echo ""
echo "Build Files:"
echo "-------------------"
for file in "CMakeLists.txt" "build.sh"; do
    if [ -f "$file" ]; then
        echo "✓ $file (exists, $(wc -l < "$file") lines)"
    else
        echo "✗ $file (missing)"
    fi
done

echo ""
echo "Summary:"
echo "-------------------"
total_dirs=16
total_cpp=13
total_h=12
total_build=2

existing_dirs=$(find . -type d | grep -v "^\.$" | grep -v "\.git" | grep -v "build" | wc -l)
existing_cpp=$(find . -name "*.cpp" | wc -l)
existing_h=$(find . -name "*.h" | wc -l)

echo "- Directories: $existing_dirs (expected $total_dirs)"
echo "- Source files (.cpp): $existing_cpp (expected $total_cpp)"
echo "- Header files (.h): $existing_h (expected $total_h)"
echo "- Build files: $([ -f "CMakeLists.txt" ] && echo 1 || echo 0) CMakeLists.txt, $([ -f "build.sh" ] && echo 1 || echo 0) build.sh"
echo ""

if [ -d "build" ]; then
    echo "Note: 'build' directory exists, which means you've run the build script."
fi

echo ""
echo "Unexpected files (if any):"
echo "-------------------"
find . -name "*.cpp" -o -name "*.h" | grep -v -f <(printf "%s\n" "${expected_cpp_files[@]}" "${expected_h_files[@]}") | grep -v "./build/"

echo ""
echo "====================================="
echo "Verification complete."