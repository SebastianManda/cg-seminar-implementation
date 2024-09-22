#include <vector>
#include <framework/disable_all_warnings.h>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
DISABLE_WARNINGS_POP()

class SurfaceMesh {
public:
    SurfaceMesh();
    SurfaceMesh(int width, int height, float size);

    void setResolution(int width, int height);
    void draw() const;

private:
    void generate();

private:
    GLuint m_vao {0};
    int m_width, m_height;
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::uvec3> m_faces;
    float m_size;
};
