cmake_minimum_required(VERSION 3.10)
project(MusicVisualizer VERSION 0.1)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set policy for GLVND
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

# Handle FFTW3 manually since FindFFTW3.cmake might not be available
find_library(FFTW3_LIBRARIES NAMES fftw3 libfftw3)
if(NOT FFTW3_LIBRARIES)
  message(FATAL_ERROR "FFTW3 library not found")
endif()

# Find libsndfile
find_library(SNDFILE_LIBRARIES NAMES sndfile libsndfile)
if(SNDFILE_LIBRARIES)
  add_definitions(-DUSE_LIBSNDFILE)
  message(STATUS "Found libsndfile: ${SNDFILE_LIBRARIES}")
else()
  message(WARNING "libsndfile not found, will use synthesized audio")
endif()

# Define USE_GLEW to switch from GLAD to GLEW
add_definitions(-DUSE_GLEW)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    /usr/include  # Add this to find fftw3.h
)

# Source files
set(SOURCES
    src/main.cpp
    src/audio/audio_manager.cpp
    src/audio/audio_buffer.cpp
    src/analysis/fft_analyzer.cpp
    src/analysis/beat_detector.cpp
    src/visualization/visualization_manager.cpp
    src/visualization/visualizer.cpp
    src/visualization/bar_visualizer.cpp
    src/visualization/wave_visualizer.cpp
    src/visualization/particle_visualizer.cpp
    src/render/render_engine.cpp
    src/render/shader_manager.cpp
    src/input/input_handler.cpp
)

# Create executable
add_executable(music_visualizer ${SOURCES})

# Link libraries
target_link_libraries(music_visualizer
    ${OPENGL_LIBRARIES}
    glfw
    ${GLEW_LIBRARIES}
    ${FFTW3_LIBRARIES}
    ${SNDFILE_LIBRARIES}
    portaudio
    pthread
    dl
    m
)

# Create assets directory in the build directory if it doesn't exist yet
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)

# Comment out the assets copy since we don't have any assets yet
# Uncomment this when you add assets
# file(COPY assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})