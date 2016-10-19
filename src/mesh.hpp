#pragma once

#define GLM_FORCE_RADIANS

#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

struct MeshData
{
    std::vector<glm::vec3> positions;
};

class Mesh
{
public:
    Mesh();

    void build_vao(MeshData&&);
    void draw() const;
private:
    static constexpr GLuint position_attr_index = 0;

    MeshData data;

    GLuint vao_id;
    GLuint position_vbo_id;
};

Mesh::Mesh()
{
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &position_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo_id);
    glVertexAttribPointer(
            position_attr_index, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void Mesh::build_vao(MeshData&& new_data)
{
    data = std::move(new_data);

    glBindVertexArray(vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo_id);
    glBufferData(
            GL_ARRAY_BUFFER,
            data.positions.size() * sizeof(glm::vec3),
            data.positions.data(),
            GL_STATIC_DRAW);
}

void Mesh::draw() const
{
    glBindVertexArray(vao_id);
    glDrawArrays(GL_TRIANGLES, 0, data.positions.size());
}
