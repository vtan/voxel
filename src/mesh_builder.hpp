#pragma once

#define GLM_FORCE_RADIANS

#include "mesh.hpp"
#include "volume.hpp"
#include "voxel.hpp"

#include <algorithm>
#include <map>
#include <vector>

#include <glm/glm.hpp>

class MeshBuilder
{
public:
    MeshData build(const Volume<Voxel>);
private:
    static const std::vector<std::pair<glm::ivec3, std::vector<glm::vec3>>>
        neighbor_dirs_with_face_vertex_positions;

    void add_positions_for_surrounding_face(
            const Volume<Voxel>&,
            glm::ivec3,
            glm::vec3,
            std::back_insert_iterator<std::vector<glm::vec3>>);
};

// The border voxels are considered neighbors and are not included in the mesh.
MeshData MeshBuilder::build(const Volume<Voxel> volume)
{
    MeshData data;
    auto pos_inserter = std::back_inserter(data.positions);

    volume.for_each_voxel_in_border(1, 1, 1, [&](auto x, auto y, auto z) {
        const glm::ivec3 current_idx(x, y, z);
        const glm::vec3 current_pos(x - 1 , y - 1, z - 1);
        const Voxel current = volume.at(current_idx);

        if (current != Voxel::empty) {
            this->add_positions_for_surrounding_face(
                    volume, current_idx, current_pos, pos_inserter);
        }
    });

    auto vec3_cmp = [](auto v1, auto v2) {
        return v1.x != v2.x ? v1.x < v2.x
            : v1.y != v2.y ? v1.y < v2.y
            : v1.z < v2.z;
    };
    std::map<glm::vec3, GLubyte, decltype(vec3_cmp)>
        vertices_with_brightness(vec3_cmp);

    volume.for_each_vertex_in_border(1, 1, 1, [&](auto x, auto y, auto z) {
        const glm::vec3 vertex(x - 1, y - 1, z - 1);
        GLubyte nonempty_neighbor_voxel_count = 0;

        for (int dz = -1; dz <= 0; ++dz) {
            for (int dy = -1; dy <= 0; ++dy) {
                for (int dx = -1; dx <= 0; ++dx) {
                    const glm::ivec3 neighbor_voxel_pos(x + dx, y + dy, z + dz);
                    const Voxel neighbor = volume.at(neighbor_voxel_pos);

                    if (neighbor != Voxel::empty) {
                        ++nonempty_neighbor_voxel_count;
                    }
                }
            }
        }

        if (nonempty_neighbor_voxel_count > 0
                && nonempty_neighbor_voxel_count < 8) {
            // Mapping values from 0 (dark) to 255 (bright).
            GLubyte brightness = (8 - nonempty_neighbor_voxel_count) << 5;
            vertices_with_brightness[vertex] = brightness;
        }
    });

    auto brightness_inserter = std::back_inserter(data.brightnesses);
    std::transform(
            data.positions.begin(),
            data.positions.end(),
            brightness_inserter,
            [&](auto v) {
                assert(vertices_with_brightness.count(v) == 1);

                return vertices_with_brightness[v];
            });

    return data;
}

void MeshBuilder::add_positions_for_surrounding_face(
        const Volume<Voxel> &volume,
        const glm::ivec3 current_idx,
        const glm::vec3 current_pos,
        std::back_insert_iterator<std::vector<glm::vec3>> pos_inserter)
{
    for (auto neighbor : neighbor_dirs_with_face_vertex_positions) {
        const auto neighbor_idx = current_idx + neighbor.first;
        if (volume.at(neighbor_idx) == Voxel::empty) {
            auto &rel_positions = neighbor.second;
            std::transform(
                    rel_positions.begin(),
                    rel_positions.end(),
                    pos_inserter,
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
