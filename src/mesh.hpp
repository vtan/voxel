#pragma once

#define GLM_FORCE_RADIANS

#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

struct MeshData
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<GLubyte> brightnesses;
};

class Mesh
{
public:
    Mesh();

    void build_vao(MeshData&&);
    void draw() const;
private:
    static constexpr GLuint position_attr_index = 0;
    static constexpr GLuint normal_attr_index = 1;
    static constexpr GLuint brightness_attr_index = 2;

    MeshData data;

    GLuint vao_id;
    GLuint position_vbo_id;
    GLuint normal_vbo_id;
    GLuint brightness_vbo_id;
};

Mesh::Mesh()
{
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glEnableVertexAttribArray(position_attr_index);
    glEnableVertexAttribArray(normal_attr_index);
    glEnableVertexAttribArray(brightness_attr_index);

    glGenBuffers(1, &position_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo_id);
    glVertexAttribPointer(
            position_attr_index, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &normal_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id);
    glVertexAttribPointer(
            normal_attr_index, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &brightness_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, brightness_vbo_id);
    glVertexAttribPointer(
            brightness_attr_index, 1, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
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

    glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id);
    glBufferData(
            GL_ARRAY_BUFFER,
            data.normals.size() * sizeof(glm::vec3),
            data.normals.data(),
            GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, brightness_vbo_id);
    glBufferData(
            GL_ARRAY_BUFFER,
            data.brightnesses.size() * sizeof(GLubyte),
            data.brightnesses.data(),
            GL_STATIC_DRAW);
}

void Mesh::draw() const
{
    glBindVertexArray(vao_id);
    glDrawArrays(GL_TRIANGLES, 0, data.positions.size());
}
