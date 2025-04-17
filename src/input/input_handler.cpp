#include <iostream>
#include <GLFW/glfw3.h>
#include "input/input_handler.h"

InputHandler::InputHandler(GLFWwindow* window)
    : m_window(window)
    , m_mouseX(0.0)
    , m_mouseY(0.0)
    , m_lastKeyPressed(-1)
    , m_keyJustPressed(false)
{
}

InputHandler::~InputHandler() {
}

bool InputHandler::initialize() {
    if (!m_window) {
        std::cerr << "Invalid GLFW window handle" << std::endl;
        return false;
    }
    
    // Initialize input state
    update();
    
    std::cout << "Input handler initialized" << std::endl;
    
    return true;
}

void InputHandler::update() {
    // Update previous key states
    m_prevKeyStates = m_keyStates;
    
    // Update previous mouse button states
    m_prevMouseButtonStates = m_mouseButtonStates;
    
    // Reset key just pressed flag
    m_keyJustPressed = false;
    
    // Get mouse position
    glfwGetCursorPos(m_window, &m_mouseX, &m_mouseY);
    
    // Check for key presses (we'll only check keys we're actually using)
    // This is more efficient than checking all possible keys
    const int keysToCheck[] = {
        GLFW_KEY_ESCAPE, GLFW_KEY_SPACE, GLFW_KEY_P,
        GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
        GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
        GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3
    };
    
    const int numKeys = sizeof(keysToCheck) / sizeof(keysToCheck[0]);
    
    for (int i = 0; i < numKeys; ++i) {
        int key = keysToCheck[i];
        int state = glfwGetKey(m_window, key);
        
        bool pressed = (state == GLFW_PRESS);
        m_keyStates[key] = pressed;
        
        // Check for just pressed keys
        if (pressed && !m_prevKeyStates[key]) {
            m_lastKeyPressed = key;
            m_keyJustPressed = true;
        }
    }
    
    // Check mouse buttons
    for (int button = GLFW_MOUSE_BUTTON_1; button <= GLFW_MOUSE_BUTTON_LAST; ++button) {
        int state = glfwGetMouseButton(m_window, button);
        m_mouseButtonStates[button] = (state == GLFW_PRESS);
    }
}

bool InputHandler::isKeyPressed(int key) const {
    auto it = m_keyStates.find(key);
    if (it != m_keyStates.end()) {
        return it->second;
    }
    return false;
}

bool InputHandler::isKeyJustPressed() const {
    return m_keyJustPressed;
}

bool InputHandler::isKeyJustReleased(int key) const {
    auto currIt = m_keyStates.find(key);
    auto prevIt = m_prevKeyStates.find(key);
    
    bool curr = (currIt != m_keyStates.end()) ? currIt->second : false;
    bool prev = (prevIt != m_prevKeyStates.end()) ? prevIt->second : false;
    
    return !curr && prev;
}

void InputHandler::getMousePosition(double& x, double& y) const {
    x = m_mouseX;
    y = m_mouseY;
}

bool InputHandler::isMouseButtonPressed(int button) const {
    auto it = m_mouseButtonStates.find(button);
    if (it != m_mouseButtonStates.end()) {
        return it->second;
    }
    return false;
}

bool InputHandler::isMouseButtonJustPressed(int button) const {
    auto currIt = m_mouseButtonStates.find(button);
    auto prevIt = m_prevMouseButtonStates.find(button);
    
    bool curr = (currIt != m_mouseButtonStates.end()) ? currIt->second : false;
    bool prev = (prevIt != m_prevMouseButtonStates.end()) ? prevIt->second : false;
    
    return curr && !prev;
}

bool InputHandler::isMouseButtonJustReleased(int button) const {
    auto currIt = m_mouseButtonStates.find(button);
    auto prevIt = m_prevMouseButtonStates.find(button);
    
    bool curr = (currIt != m_mouseButtonStates.end()) ? currIt->second : false;
    bool prev = (prevIt != m_prevMouseButtonStates.end()) ? prevIt->second : false;
    
    return !curr && prev;
}