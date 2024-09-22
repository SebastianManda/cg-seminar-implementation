#include "camera.h"
#include "imgui/imgui.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>

DISABLE_WARNINGS_PUSH()

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

DISABLE_WARNINGS_POP()

Camera::Camera(Window* pWindow, const glm::vec3 &pos, const glm::vec3 &forward)
        : position(pos), forward(glm::normalize(forward)), m_pWindow(pWindow) {
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position, position + forward, up);
}

void Camera::zoom(float z) {
    const float zoomSpeed = 5.0f;
    fov -= z * zoomSpeed;
}

void Camera::rotateX(float angle) {
    const glm::vec3 horAxis = glm::cross(s_yAxis, forward);

    forward = glm::normalize(glm::angleAxis(angle, horAxis) * forward);
    up = glm::normalize(glm::cross(forward, horAxis));
}

void Camera::rotateY(float angle) {
    const glm::vec3 horAxis = glm::cross(s_yAxis, forward);

    forward = glm::normalize(glm::angleAxis(angle, s_yAxis) * forward);
    up = glm::normalize(glm::cross(forward, horAxis));
}

void Camera::updateInput() {
    constexpr float moveSpeed = 0.02f;
    constexpr float lookSpeed = 0.0015f;

    glm::vec3 localMoveDelta{0};
    const glm::vec3 right = glm::normalize(glm::cross(forward, up));
    if (m_pWindow->isKeyPressed(GLFW_KEY_A))
        position -= moveSpeed * right;
    if (m_pWindow->isKeyPressed(GLFW_KEY_D))
        position += moveSpeed * right;
    if (m_pWindow->isKeyPressed(GLFW_KEY_W))
        position += moveSpeed * forward;
    if (m_pWindow->isKeyPressed(GLFW_KEY_S))
        position -= moveSpeed * forward;
    if (m_pWindow->isKeyPressed(GLFW_KEY_R))
        position += moveSpeed * up;
    if (m_pWindow->isKeyPressed(GLFW_KEY_F))
        position -= moveSpeed * up;

    const glm::dvec2 cursorPos = m_pWindow->getCursorPos();
    const glm::vec2 delta = lookSpeed * glm::vec2(cursorPos - m_prevCursorPos);
    m_prevCursorPos = cursorPos;

    if (ImGui::GetIO().WantCaptureMouse) return;

    if (m_pWindow->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        if (delta.x != 0.0f)
            rotateY(delta.x);
        if (delta.y != 0.0f)
            rotateX(delta.y);
    }
}