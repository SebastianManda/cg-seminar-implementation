//#include "Image.h"
#include "mesh.h"
#include "texture.h"
// Always include window first (because it includes glfw, which includes GL which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include <framework/disable_all_warnings.h>
#include <framework/trackball.h>

#include "terrain/surface_mesh.h"

DISABLE_WARNINGS_PUSH()

#include <glad/glad.h>
// Include glad before glfw3
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <imgui/imgui.h>

DISABLE_WARNINGS_POP()

#include <framework/shader.h>
#include <framework/window.h>
#include <functional>
#include <iostream>
#include <vector>

class Application {
public:
    Application()
            : m_window("CG Seminar Implementation", glm::ivec2(1600, 900), OpenGLVersion::GL45),
              m_texture("resources/phasorNoise.png"),
              m_trackballCamera(&m_window, glm::radians(90.0f)) {

        m_window.registerWindowResizeCallback([&](const glm::ivec2& size) {
            glViewport(0, 0, size.x, size.y);
        });
        // m_window.registerKeyCallback([this](int key, int scancode, int action, int mods) {
        //     if (action == GLFW_PRESS)
        //         onKeyPressed(key, mods);
        //     else if (action == GLFW_RELEASE)
        //         onKeyReleased(key, mods);
        // });
        // m_window.registerMouseMoveCallback(std::bind(&Application::onMouseMove, this, std::placeholders::_1));
        // m_window.registerMouseButtonCallback([this](int button, int action, int mods) {
        //     if (action == GLFW_PRESS)
        //         onMouseClicked(button, mods);
        //     else if (action == GLFW_RELEASE)
        //         onMouseReleased(button, mods);
        // });
        // m_window.registerScrollCallback([&](glm::vec2 offset) {
        //     m_camera.zoom(offset.y);
        // });

        m_meshes = GPUMesh::loadMeshGPU("resources/dragon.obj");

        try {
            ShaderBuilder defaultBuilder;
            // defaultBuilder.addStage(GL_VERTEX_SHADER, "shaders/shader_vert.glsl");
            defaultBuilder.addStage(GL_VERTEX_SHADER, "shaders/surface_vert.glsl");
            defaultBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/shader_frag.glsl");
            m_defaultShader = defaultBuilder.build();

            ShaderBuilder shadowBuilder;
            shadowBuilder.addStage(GL_VERTEX_SHADER, "shaders/shadow_vert.glsl");
            m_shadowShader = shadowBuilder.build();
        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void update() {
        while (!m_window.shouldClose()) {
            m_window.updateInput();
            gui();

            m_projectionMatrix = m_trackballCamera.projectionMatrix();
            m_viewMatrix = m_trackballCamera.viewMatrix();
            m_surfaceMesh.update();

            // Clear the screen
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            const glm::mat4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

            // Render meshes
            for (GPUMesh &mesh: m_meshes) {
                m_defaultShader.bind();
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
                m_texture.bind(GL_TEXTURE0);
                glUniform1i(3, 0);
                glUniform1f(4, heightScale);
                glUniform1f(5, m_surfaceMesh.m_size);

                m_surfaceMesh.draw();
            }

            m_window.swapBuffers();
        }
    }

    void gui() {
        ImGui::Begin("Window");
        ImGui::Checkbox("Camera can translate", &m_trackballCamera.m_canTranslate);
        ImGui::Text("Surface Mesh Options");
        ImGui::Checkbox("Filled", &m_surfaceMesh.m_filled);
        ImGui::SliderFloat("Height Scale", &heightScale, 0.01f, 1.0f);
        ImGui::SliderFloat("Mesh Size", &m_surfaceMesh.m_size, 1.0f, 50.0f);
        ImGui::DragInt("Vertex resoluition", &m_surfaceMesh.m_resolution, 1, 2, 2000);
        ImGui::End();
    }

    // void onKeyPressed(int key, int mods) { std::cout << "Key pressed: " << key << std::endl; }
    // void onKeyReleased(int key, int mods) { std::cout << "Key released: " << key << std::endl; }
    // void onMouseMove(const glm::dvec2 &cursorPos) { std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl; }
    // void onMouseClicked(int button, int mods) { std::cout << "Pressed mouse button: " << button << std::endl; }
    // void onMouseReleased(int button, int mods) { std::cout << "Released mouse button: " << button << std::endl; }

private:
    Window m_window;
    Trackball m_trackballCamera;

    // Shader for default rendering and for depth rendering
    Shader m_defaultShader;
    Shader m_shadowShader;

    std::vector<GPUMesh> m_meshes;
    Texture m_texture;
    bool m_useMaterial{true};

    SurfaceMesh m_surfaceMesh;
    float heightScale = 1.0f;

    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 30.0f);
    glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix{1.0f};
};

int main() {
    Application app;
    app.update();

    return 0;
}
