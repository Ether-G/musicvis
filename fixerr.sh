#!/bin/bash

echo "Fixing render_engine.cpp to use GLEW instead of GLAD..."

# Create a backup
cp src/render/render_engine.cpp src/render/render_engine.cpp.bak

# Replace GLAD initialization code with GLEW
sed -i 's/if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {/if (glewInit() != GLEW_OK) {/' src/render/render_engine.cpp

# Replace any other glad functions if needed
sed -i 's/gladLoadGL/glewInit/g' src/render/render_engine.cpp

echo "Fixed render_engine.cpp!"

# Check for any remaining references to glad
if grep -q "glad" src/render/render_engine.cpp; then
    echo "Warning: There are still references to 'glad' in the file:"
    grep -n "glad" src/render/render_engine.cpp
else
    echo "No more references to 'glad' found."
fi

echo "Done!"