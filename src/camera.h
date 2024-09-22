// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/window.h>

class Camera {
public:
    Camera(Window* pWindow, const glm::vec3& position, const glm::vec3& forward);

    void updateInput();
    glm::mat4 viewMatrix() const;
    void zoom(float z);
    void rotateX(float angle);
    void rotateY(float angle);

public:
    glm::vec3 position { 0 };
    glm::vec3 forward { 0, 0, -1 };
    glm::vec3 up { 0, 1, 0 };
    float fov = 70.0f;
    float zNear = 0.1f;
    float zFar = 30.0f;

private:
    static constexpr glm::vec3 s_yAxis { 0, 1, 0 };
    const Window* m_pWindow;
    glm::dvec2 m_prevCursorPos { 0 };
};
