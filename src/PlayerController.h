#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class PlayerController {
public:
    glm::vec3 position{0.0f};
    float yaw   {0.0f}; // degrees
    float pitch {0.0f}; // degrees
    float speed {5.0f};

    void update(GLFWwindow* window, float deltaTime);
    glm::mat4 getViewMatrix() const;
};