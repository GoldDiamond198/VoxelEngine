#include "PlayerController.h"

void PlayerController::update(GLFWwindow* window, float dt) {
    glm::vec3 forward{
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    };
    forward = glm::normalize(forward);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.f, 1.f, 0.f)));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    glm::vec3 move{0.f};
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= forward;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += right;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= right;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) move += up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) move -= up;
    if (glm::length(move) > 0.f)
        position += glm::normalize(move) * speed * dt;

    const float turnSpeed = 90.f; // degrees per second
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  yaw -= turnSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw += turnSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    pitch += turnSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  pitch -= turnSpeed * dt;

    if (pitch > 89.f) pitch = 89.f;
    if (pitch < -89.f) pitch = -89.f;
}

glm::mat4 PlayerController::getViewMatrix() const {
    glm::vec3 dir{
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    };
    return glm::lookAt(position, position + dir, glm::vec3(0.f, 1.f, 0.f));
}
