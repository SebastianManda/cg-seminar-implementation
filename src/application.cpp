//#include "Image.h"
#include "mesh.h"
#include "texture.h"
// Always include window first (because it includes glfw, which includes GL which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include <queue>
#include <framework/disable_all_warnings.h>
#include <framework/trackball.h>

#include "helpers/amplitude.h"
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
              m_trackballCamera(&m_window, glm::radians(90.0f)),
              m_terrain1("resources/terrains/terrain_test_HR.png"),
              m_terrain2("resources/terrains/zoom_appalache_20km.png"),
              m_amplitude1(m_terrain1.getData()),
              m_amplitude2(m_terrain2.getData()) {

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

        // INIT MAPS
        m_amplitudeMap.Init();

        try {
            ShaderBuilder defaultBuilder;
            // defaultBuilder.addStage(GL_VERTEX_SHADER, "shaders/shader_vert.glsl");
            defaultBuilder.addStage(GL_VERTEX_SHADER, "shaders/surface_vert.glsl");
            defaultBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/shader_frag.glsl");
            m_defaultShader = defaultBuilder.build();

            ShaderBuilder texBuilder;
            texBuilder.addStage(GL_VERTEX_SHADER, "shaders/quad_vert.glsl");
            texBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/texture_frag.glsl");
            m_textureShader = texBuilder.build();

            ShaderBuilder amplitudeBuilder;
            amplitudeBuilder.addStage(GL_VERTEX_SHADER, "shaders/quad_vert.glsl");
            amplitudeBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/amplitude_frag.glsl");
            m_amplitudeShader = amplitudeBuilder.build();


            // ShaderBuilder shadowBuilder;
            // shadowBuilder.addStage(GL_VERTEX_SHADER, "shaders/shadow_vert.glsl");
            // m_shadowShader = shadowBuilder.build();
        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void preprocessing() {
        m_amplitude1.process();
        m_amplitude2.process();
    }

    void update() {
        while (!m_window.shouldClose()) {
            m_window.updateInput();
            gui();

            m_projectionMatrix = m_trackballCamera.projectionMatrix();
            m_viewMatrix = m_trackballCamera.viewMatrix();
            m_surfaceMesh.update();

            const glm::mat4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

            // Render amplitude map
            m_amplitudeMap.bindWrite();
            m_amplitudeShader.bind();
            m_terrain1.bind(GL_TEXTURE0);
            glUniform1i(1, 0);
            drawEmpty();

            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glViewport( 0, 0, m_window.getWindowSize().x, m_window.getWindowSize().y);
            glClearColor(0.57f, 0.76f, 0.98f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            // Render mesh
            m_defaultShader.bind();
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
            glUniform3fv(4, 1, glm::value_ptr(glm::vec3(m_heightScale, m_meshScale, m_useColor)));
            glUniform3fv(5, 1, glm::value_ptr(m_trackballCamera.position()));
            glUniform3fv(7, 1, glm::value_ptr(glm::vec3(m_useRivers, m_riversMult, m_riversThreshold)));
            glUniform1i(8, m_useAmplitude);

            if (m_currentTerrain == 0) {
                m_terrain1.bind(GL_TEXTURE0);
                glUniform1i(3, 0);
                if (m_useRivers) m_amplitude1.m_drainageMap.bindRead(GL_TEXTURE1);
                if (m_useAmplitude) m_amplitude1.m_amplitudeMap.bindRead(GL_TEXTURE1);
                glUniform1i(6, 1);
            } else {
                m_terrain2.bind(GL_TEXTURE0);
                glUniform1i(3, 0);
                if (m_useRivers) m_amplitude2.m_drainageMap.bindRead(GL_TEXTURE1);
                if (m_useAmplitude) m_amplitude2.m_amplitudeMap.bindRead(GL_TEXTURE1);
                glUniform1i(6, 1);
            }

            m_surfaceMesh.draw();

            m_window.swapBuffers();
        }
    }

    void gui() {
        ImGui::Begin("Options");
        ImGui::Checkbox("Camera can translate", &m_trackballCamera.m_canTranslate);
        ImGui::Text("Terrain Options");
        ImGui::RadioButton("Terrain 1", &m_currentTerrain, 0);
        ImGui::RadioButton("Terrain 2", &m_currentTerrain, 1);
        ImGui::Text("Surface Mesh Options");
        ImGui::Checkbox("Color", &m_useColor);
        ImGui::Checkbox("Filled", &m_surfaceMesh.m_filled);
        ImGui::SliderFloat("Height Scale", &m_heightScale, 0.0f, 2.0f);
        ImGui::SliderFloat("Mesh Size", &m_meshScale, 1.0f, 20.0f);
        ImGui::DragInt("Vertex resoluition", &m_surfaceMesh.m_resolution, 1, 2, 2000);
        ImGui::Text("Visualisation Options");
        ImGui::Checkbox("Rivers", &m_useRivers);
        ImGui::SliderFloat("Rivers Mult", &m_riversMult, 0.0f, 10.0f);
        ImGui::SliderFloat("Rivers Threshold", &m_riversThreshold, 0.0f, 2.0f);
        ImGui::Checkbox("Amplitude", &m_useAmplitude);
        ImGui::InputFloat("Amplitude Increment", &m_amplitudeIncr);
        if (ImGui::Button("Recompute Amplitude")) {
            if (m_currentTerrain == 0) m_amplitude1.compute(m_riversThreshold, m_amplitudeIncr);
            else m_amplitude2.compute(m_riversThreshold, m_amplitudeIncr);
        }
        ImGui::End();
    }

    void drawEmpty() {
        GLuint emptyVAO;
        glGenVertexArrays(1, &emptyVAO);
        glBindVertexArray(emptyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
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
    Shader m_textureShader;
    Shader m_amplitudeShader;
    Shader m_shadowShader;

    Texture m_terrain1;
    Texture m_terrain2;
    Amplitude m_amplitude1;
    Amplitude m_amplitude2;

    TextureMap m_amplitudeMap;

    SurfaceMesh m_surfaceMesh;
    float m_heightScale{1.0f};
    float m_meshScale{2.5f};
    bool m_useColor{false};
    int m_currentTerrain{0};
    bool m_useRivers{false};
    float m_riversMult{2.0f};
    float m_riversThreshold{0.6f};
    bool m_useAmplitude{false};
    float m_amplitudeIncr{0.04f};

    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 30.0f);
    glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix{1.0f};
};

int main() {
    Application app;
    app.preprocessing();
    app.update();

    return 0;
}
