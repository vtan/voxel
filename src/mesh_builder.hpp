#pragma once

#define GLM_FORCE_RADIANS

#include "mesh.hpp"
#include "volume.hpp"
#include "voxel.hpp"

#include <algorithm>
#include <vector>

#include <glm/glm.hpp>

class MeshBuilder
{
public:
    MeshBuilder(Mesh &m) : mesh(m) {}

    void build(const Volume<Voxel>);
private:
    static const std::vector<std::pair<glm::ivec3, std::vector<glm::vec3>>>
        neighbor_dirs_with_face_vertex_positions;

    Mesh &mesh;

    void add_positions_for_surrounding_face(
            const Volume<Voxel>&, glm::ivec3, glm::vec3);
};

// The border voxels are considered neighbors and are not included in the mesh.
void MeshBuilder::build(const Volume<Voxel> volume)
{
    auto &positions = mesh.get_positions();
    positions.clear();

    volume.for_each_in_border(1, 1, 1, [&](auto x, auto y, auto z) {
        const glm::ivec3 current_idx(x, y, z);
        const glm::vec3 current_pos(x - 1 , y - 1, z - 1);
        const Voxel current = volume.at(current_idx);

        if (current != Voxel::empty) {
            this->add_positions_for_surrounding_face(
                    volume, current_idx, current_pos);
        }
    });
}

void MeshBuilder::add_positions_for_surrounding_face(
        const Volume<Voxel> &volume,
        const glm::ivec3 current_idx,
        const glm::vec3 current_pos)
{
    auto &positions = mesh.get_positions();

    for (auto neighbor : neighbor_dirs_with_face_vertex_positions) {
        const auto neighbor_idx = current_idx + neighbor.first;
        if (volume.at(neighbor_idx) == Voxel::empty) {
            auto &rel_positions = neighbor.second;
            std::transform(
                    rel_positions.begin(),
                    rel_positions.end(),
                    std::back_inserter(positions),
                    [=](auto rel_pos) { return current_pos + rel_pos; });
        }
    }
}

const std::vector<std::pair<glm::ivec3, std::vector<glm::vec3>>>
    MeshBuilder::neighbor_dirs_with_face_vertex_positions = {
    // Left face
    { glm::ivec3(-1, 0, 0),
        {
            glm::vec3(0, 0, 0),
            glm::vec3(0, 0, 1),
            glm::vec3(0, 1, 0),
            glm::vec3(0, 0, 1),
            glm::vec3(0, 1, 1),
            glm::vec3(0, 1, 0),
        }
    },

    // Right face
    { glm::ivec3(1, 0, 0),
        {
            glm::vec3(1, 0, 0),
            glm::vec3(1, 1, 0),
            glm::vec3(1, 0, 1),
            glm::vec3(1, 0, 1),
            glm::vec3(1, 1, 0),
            glm::vec3(1, 1, 1),
        }
    },

    // Bottom face
    { glm::ivec3(0, -1, 0),
        {
            glm::vec3(0, 0, 0),
            glm::vec3(1, 0, 0),
            glm::vec3(0, 0, 1),
            glm::vec3(1, 0, 0),
            glm::vec3(1, 0, 1),
            glm::vec3(0, 0, 1),
        }
    },

    // Top face
    { glm::ivec3(0, 1, 0),
        {
            glm::vec3(0, 1, 0),
            glm::vec3(0, 1, 1),
            glm::vec3(1, 1, 0),
            glm::vec3(1, 1, 0),
            glm::vec3(0, 1, 1),
            glm::vec3(1, 1, 1),
        }
    },

    // Back face
    { glm::ivec3(0, 0, -1),
        {
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0),
            glm::vec3(1, 0, 0),
            glm::vec3(0, 1, 0),
            glm::vec3(1, 1, 0),
            glm::vec3(1, 0, 0),
        }
    },


    // Front face
    { glm::ivec3(0, 0, 1),
        {
            glm::vec3(0, 0, 1),
            glm::vec3(1, 0, 1),
            glm::vec3(0, 1, 1),
            glm::vec3(0, 1, 1),
            glm::vec3(1, 0, 1),
            glm::vec3(1, 1, 1),
        }
    },
};
