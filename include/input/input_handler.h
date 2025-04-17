#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <unordered_map>

struct GLFWwindow;

class InputHandler {
public:
    InputHandler(GLFWwindow* window);
    ~InputHandler();

    // Initialize the input handler
    bool initialize();
    
    // Update input state
    void update();
    
    // Check if a key is currently pressed
    bool isKeyPressed(int key) const;
    
    // Check if a key was just pressed this frame
    bool isKeyJustPressed() const;
    
    // Check if a key was just released this frame
    bool isKeyJustReleased(int key) const;
    
    // Get mouse position
    void getMousePosition(double& x, double& y) const;
    
    // Check if a mouse button is currently pressed
    bool isMouseButtonPressed(int button) const;
    
    // Check if a mouse button was just pressed this frame
    bool isMouseButtonJustPressed(int button) const;
    
    // Check if a mouse button was just released this frame
    bool isMouseButtonJustReleased(int button) const;

private:
    // GLFW window
    GLFWwindow* m_window;
    
    // Current key states
    std::unordered_map<int, bool> m_keyStates;
    
    // Previous key states
    std::unordered_map<int, bool> m_prevKeyStates;
    
    // Current mouse button states
    std::unordered_map<int, bool> m_mouseButtonStates;
    
    // Previous mouse button states
    std::unordered_map<int, bool> m_prevMouseButtonStates;
    
    // Mouse position
    double m_mouseX;
    double m_mouseY;
    
    // Key that was just pressed (for key repeat handling)
    int m_lastKeyPressed;
    bool m_keyJustPressed;
};

#endif // INPUT_HANDLER_H