#include <vector>
#include <framework/disable_all_warnings.h>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
DISABLE_WARNINGS_POP()

class SurfaceMesh {
public:
    SurfaceMesh();
    SurfaceMesh(int resolution, float size);

    void update();
    void draw() const;

public:
    int m_resolution {400};
    float m_size {2.0f};
    bool m_filled {false};

private:
    void generate();
    void createVertices(float y);
    void createFaces(int startingIndex);
    void fillSurface();

private:
    GLuint m_vao {0};
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::uvec3> m_faces;
    int previous_resolution;
    float previous_size;
    bool previous_filled {false};
};
