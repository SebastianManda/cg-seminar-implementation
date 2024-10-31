//#include "Image.h"
#include "mesh.h"
#include "texture.h"
// Always include window first (because it includes glfw, which includes GL which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include <queue>
#include <framework/disable_all_warnings.h>
#include <framework/trackball.h>

#include "helpers/amplitude.h"
#include "helpers/orientation.h"
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

struct terrainStats {
    float heightScale;
    float meshScale;
    float riversMult;
    float riversThreshold;
    float amplitudeIncr;
    float detailModifier;
    float frequency;
    float epsilon;
    float C;
    float p_complement;
};

class Application {
public:
    Application()
            : m_window("CG Seminar Implementation", glm::ivec2(1600, 900), OpenGLVersion::GL45),
              m_trackballCamera(&m_window, glm::radians(90.0f)),
              m_terrain1("resources/terrains/terrain_test_HR.png"),
              m_terrain2("resources/terrains/zoom_appalache_20km.png"),
              m_terrain3("resources/terrains/vf_moldoveanu.png"),
              m_upscaledTerrain1("resources/terrains/upscale_inter_1.png"),
              m_upscaledTerrain2("resources/terrains/upscale_inter_2.png"),
              m_upscaledTerrain3("resources/terrains/vf_moldoveanu_upscale.png"),
              m_externalAmplitude1("resources/smile.png"),
              m_externalAmplitude2("resources/checkerboard.png"),
              m_amplitude1(m_terrain1.getData()),
              m_amplitude2(m_terrain2.getData()),
              m_amplitude3(m_terrain3.getData()),
              m_orientation1(m_upscaledTerrain1.getData()),
              m_orientation2(m_upscaledTerrain2.getData()),
              m_orientation3(m_upscaledTerrain3.getData()) {

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
        m_phasorNoise.Init();
        m_currentTexture = &m_phasorNoise;
        m_detailsMap_Phasor.Init();
        m_elevationMap.Init();
        m_stats = &m_terrainStats1;
        m_amplitude = &m_amplitude1;
        m_orientation = &m_orientation1;
        m_terrain = &m_terrain1;
        m_smoothTerrain = &m_upscaledTerrain1;
        m_externalAmplitude = &m_externalAmplitude1;

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

            ShaderBuilder phasorBuilder;
            phasorBuilder.addStage(GL_VERTEX_SHADER, "shaders/quad_vert.glsl");
            phasorBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/phasor_frag.glsl");
            m_phasorShader = phasorBuilder.build();

            ShaderBuilder detailsBuilder;
            detailsBuilder.addStage(GL_VERTEX_SHADER, "shaders/quad_vert.glsl");
            detailsBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/details_frag.glsl");
            m_detailsShader = detailsBuilder.build();

            ShaderBuilder elevationBuilder;
            elevationBuilder.addStage(GL_VERTEX_SHADER, "shaders/quad_vert.glsl");
            elevationBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/elevation_frag.glsl");
            m_elevationShader = elevationBuilder.build();

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
        m_amplitude3.process();
        m_orientation1.process();
        m_orientation2.process();
        m_orientation3.process();
    }

    void setCurrentValues() {
        m_terrain = (m_currentTerrain == 0) ? &m_terrain1 : (m_currentTerrain == 1) ? &m_terrain2 : &m_terrain3;
        m_amplitude = (m_currentTerrain == 0) ? &m_amplitude1 : (m_currentTerrain == 1) ? &m_amplitude2 : &m_amplitude3;
        m_orientation = (m_currentTerrain == 0) ? &m_orientation1 : (m_currentTerrain == 1) ? &m_orientation2 : &m_orientation3;
        m_smoothTerrain = (m_currentTerrain == 0) ? &m_upscaledTerrain1 : (m_currentTerrain == 1) ? &m_upscaledTerrain2 : &m_upscaledTerrain3;
        m_stats = (m_currentTerrain == 0) ? &m_terrainStats1 : (m_currentTerrain == 1) ? &m_terrainStats2 : &m_terrainStats3;
        m_externalAmplitude = (m_currentAmplitude == 0) ? &m_externalAmplitude1 : &m_externalAmplitude2;

        if (m_useRivers) m_currentTexture = &m_amplitude->m_drainageMap;
        if (m_useAmplitude) m_currentTexture = &m_amplitude->m_amplitudeMap;
        if (m_useGradient) m_currentTexture = &m_orientation->m_gradientMap;
        if (m_useOrientation) m_currentTexture = &m_orientation->m_orientationMap;
        if (m_usePhasor) m_currentTexture = &m_phasorNoise;
        if (m_useR) m_currentTexture = &m_detailsMap_Phasor;
    }

    void update() {
        while (!m_window.shouldClose()) {
            m_window.updateInput();
            gui();

            m_projectionMatrix = m_trackballCamera.projectionMatrix();
            m_viewMatrix = m_trackballCamera.viewMatrix();
            m_surfaceMesh.update();
            setCurrentValues();

            const glm::mat4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

            // Render noise map
            m_phasorNoise.bindWrite();
            m_phasorShader.bind();
            m_orientation->m_orientationMap.bindRead(GL_TEXTURE0);
            glUniform1i(1, 0);
            glUniform1f(2, m_stats->frequency / 100.0f);
            // Temp
            glUniform1i(11, toggle_ang_profile);
            glUniform1f(12, m_stats->epsilon / 100.0f);
            glUniform1i(13, toggle_rad_attenuation);
            glUniform1f(14, m_stats->C);
            glUniform1i(15, toggle_transform);
            glUniform1f(16, m_stats->p_complement / 100.0f);
            drawEmpty();

            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            // Render details map
            m_detailsMap_Phasor.bindWrite();
            m_detailsShader.bind();
            if (m_useExternalAmplitude) m_externalAmplitude->bind(GL_TEXTURE0);
            else m_amplitude->m_amplitudeMap.bindRead(GL_TEXTURE0);
            glUniform1i(1, 0);
            m_phasorNoise.bindRead(GL_TEXTURE1);
            glUniform1i(2, 1);
            drawEmpty();

            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            // Render elevation map
            m_elevationMap.bindWrite();
            m_elevationShader.bind();
            m_smoothTerrain->bind(GL_TEXTURE0);
            glUniform1i(1, 0);
            m_detailsMap_Phasor.bindRead(GL_TEXTURE1);
            glUniform1i(2, 1);
            glUniform1f(3, m_stats->detailModifier / 1000.0f);
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
            glUniform4fv(4, 1, glm::value_ptr(glm::vec4(m_stats->heightScale, m_stats->meshScale, m_surfaceMesh.m_resolution, m_useColor)));
            glUniform3fv(5, 1, glm::value_ptr(m_trackballCamera.position()));
            glUniform3fv(7, 1, glm::value_ptr(glm::vec3(m_useRivers, m_stats->riversMult, m_stats->riversThreshold)));
            glUniform1i(8, m_useAmplitude || m_showExternalAmplitude || m_useGradient || m_usePhasor || m_useR);
            glUniform1i(9, m_useOrientation);
            // m_terrain->bind(GL_TEXTURE0);
            if (use_upscaled) m_smoothTerrain->bind(GL_TEXTURE0);
            else m_terrain->bind(GL_TEXTURE0);
            if (m_finalElevation) m_elevationMap.bindRead(GL_TEXTURE0);
            glUniform1i(3, 0);
            if (m_showExternalAmplitude) m_externalAmplitude->bind(GL_TEXTURE1);
            else m_currentTexture->bindRead(GL_TEXTURE1);
            glUniform1i(6, 1);
            m_surfaceMesh.draw();

            m_window.swapBuffers();
        }
    }

    void gui() {
        ImGui::Begin("Options");
        ImGui::Checkbox("Use Final Elevation", &m_finalElevation);
        ImGui::Checkbox("Use Details Map", &m_useR);
        ImGui::Checkbox("Use Upscaled Map", &use_upscaled);
        ImGui::Checkbox("Camera can translate", &m_trackballCamera.m_canTranslate);
        ImGui::Text("Terrain Options");
        ImGui::RadioButton("Terrain 1", &m_currentTerrain, 0);
        ImGui::RadioButton("Terrain 2", &m_currentTerrain, 1);
        ImGui::RadioButton("Terrain 3", &m_currentTerrain, 2);
        ImGui::Text("Surface Mesh Options");
        ImGui::Checkbox("Color", &m_useColor);
        ImGui::Checkbox("Filled", &m_surfaceMesh.m_filled);
        ImGui::SliderFloat("Height Scale", &m_stats->heightScale, 0.0f, 2.0f);
        ImGui::SliderFloat("Mesh Size", &m_stats->meshScale, 1.0f, 20.0f);
        ImGui::DragInt("Vertex resoluition", &m_surfaceMesh.m_resolution, 1, 2, 2000);
        ImGui::Text("Visualisation Options");
        ImGui::Checkbox("Show External Amplitude", &m_showExternalAmplitude);
        ImGui::Checkbox("Use External Amplitude", &m_useExternalAmplitude);
        ImGui::RadioButton("Smile", &m_currentAmplitude, 0);
        ImGui::RadioButton("Checkerboard", &m_currentAmplitude, 1);
        ImGui::Checkbox("Rivers", &m_useRivers);
        ImGui::SliderFloat("Rivers Mult", &m_stats->riversMult, 0.0f, 10.0f);
        ImGui::SliderFloat("Rivers Threshold", &m_stats->riversThreshold, 0.0f, 2.0f);
        ImGui::Checkbox("Amplitude", &m_useAmplitude);
            ImGui::InputFloat("Amplitude Increment", &m_stats->amplitudeIncr);
        if (ImGui::Button("Recompute Amplitude")) {
            if (m_currentTerrain == 0) m_amplitude1.compute(m_stats->riversThreshold, m_stats->amplitudeIncr);
            else if (m_currentTerrain == 1) m_amplitude2.compute(m_stats->riversThreshold, m_stats->amplitudeIncr);
            else if (m_currentTerrain == 2) m_amplitude3.compute(m_stats->riversThreshold, m_stats->amplitudeIncr);
        }
        ImGui::Checkbox("Gradient", &m_useGradient);
        ImGui::Checkbox("Orientation", &m_useOrientation);
        ImGui::Text("Phasor Noise Options");
        ImGui::Checkbox("Phasor Noise", &m_usePhasor);
        ImGui::DragFloat("Frequency", &m_stats->frequency, 1, 0.0f, 100.0f);
        ImGui::Checkbox("Toggle angular profile", &toggle_ang_profile);
        ImGui::DragFloat("Epsilon", &m_stats->epsilon, 1, 0.0f, 100.0f);
        ImGui::Checkbox("Toggle radial attenuation", &toggle_rad_attenuation);
        ImGui::DragFloat("C", &m_stats->C, 1, 0.0f, 100.0f);
        ImGui::Checkbox("Toggle transform function", &toggle_transform);
        ImGui::DragFloat("p_complement", &m_stats->p_complement, 1, 0.0f, 100.0f);
        ImGui::DragFloat("Detail Modifier", &m_stats->detailModifier, 1, 0.0f, 1000.0f);
        if (ImGui::Button("Reset Terrain Options")) {
            if (m_currentTerrain == 0)
                *m_stats = terrainStats(1.0f, 2.5f, 2.0f, 0.6f, 0.04f, 40.0f, 1.0f, 1.0f, 25.0f);
            else if (m_currentTerrain == 1)
                *m_stats = terrainStats(0.25f, 2.5f, 2.0f, 0.6f, 0.04f, 100.0f, 2.0f, 1.0f, 30.0f);
            else if (m_currentTerrain == 2)
                *m_stats = terrainStats(1.0f, 2.5f, 2.0f, 0.6f, 0.04f, 40.0f, 1.0f, 1.0f, 25.0f);
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
    Shader m_phasorShader;
    Shader m_detailsShader;
    Shader m_elevationShader;
    // Shader m_shadowShader;

    Texture* m_terrain;
    Texture m_terrain1;
    Texture m_terrain2;
    Texture m_terrain3;
    Texture* m_smoothTerrain;
    Texture m_upscaledTerrain1;
    Texture m_upscaledTerrain2;
    Texture m_upscaledTerrain3;
    Texture* m_externalAmplitude;
    Texture m_externalAmplitude1;
    Texture m_externalAmplitude2;


    Amplitude* m_amplitude;
    Amplitude m_amplitude1;
    Amplitude m_amplitude2;
    Amplitude m_amplitude3;

    Orientation* m_orientation;
    Orientation m_orientation1;
    Orientation m_orientation2;
    Orientation m_orientation3;

    TextureMap* m_currentTexture;
    TextureMap m_phasorNoise{TextureMap(glm::ivec2( 2048))};
    TextureMap m_detailsMap_Phasor{TextureMap(glm::ivec2( 2048))};
    TextureMap m_elevationMap{TextureMap(glm::ivec2( 2048))};

    SurfaceMesh m_surfaceMesh{SurfaceMesh(800)};

    bool m_useColor{false};
    bool m_useRivers{false};
    bool m_useAmplitude{false};
    bool m_useGradient{false};
    bool m_useOrientation{false};
    bool m_usePhasor{false};
    bool m_useR{false};
    bool m_finalElevation{false};
    bool use_upscaled{true};
    bool toggle_ang_profile{false};
    bool toggle_rad_attenuation{false};
    bool toggle_transform{true};
    bool m_useExternalAmplitude{false};
    bool m_showExternalAmplitude{false};

    int m_currentAmplitude{0};
    int m_currentTerrain{0};
    terrainStats* m_stats;
    terrainStats m_terrainStats1 = {1.0f, 2.5f, 2.0f, 0.6f, 0.04f, 40.0f, 1.0f, 1.0f, 25.0f};
    terrainStats m_terrainStats2 = {0.25f, 2.5f, 2.0f, 0.6f, 0.04f, 100.0f, 2.0f, 1.0f, 30.0f};
    terrainStats m_terrainStats3 = {1.0f, 2.5f, 2.0f, 0.6f, 0.04f, 40.0f, 1.0f, 1.0f, 25.0f};

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
