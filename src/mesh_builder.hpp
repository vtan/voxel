#pragma once

#define GLM_FORCE_RADIANS

#include "mesh.hpp"
#include "volume.hpp"
#include "voxel.hpp"

#include <algorithm>
#include <map>
#include <vector>

#include <glm/glm.hpp>

struct Vec3Comparator
{
    bool operator()(const glm::vec3& v1, const glm::vec3& v2) const
    {
        return v1.x != v2.x ? v1.x < v2.x
            : v1.y != v2.y ? v1.y < v2.y
            : v1.z < v2.z;
    }
};

class MeshBuilder
{
public:
    MeshData build(const Volume<Voxel>);
private:
    MeshData mesh_data;
    std::map<glm::vec3, GLubyte, Vec3Comparator> vertices_with_brightness;

    void non_empty_voxel(const Volume<Voxel>&, glm::ivec3, glm::vec3);
    void vertex(const Volume<Voxel>&, glm::ivec3);

    static const std::vector<std::pair<glm::ivec3, std::vector<glm::vec3>>>
        neighbor_dirs_with_face_vertex_positions;
};

// The border voxels are considered neighbors and are not included in the mesh.
MeshData MeshBuilder::build(const Volume<Voxel> volume)
{
    mesh_data = {};
    vertices_with_brightness = {};

    volume.for_each_voxel_in_border(1, 1, 1, [&](auto x, auto y, auto z) {
        const glm::ivec3 current_idx(x, y, z);
        const glm::vec3 current_pos(x - 1 , y - 1, z - 1);
        const Voxel current = volume.at(current_idx);

        if (current != Voxel::empty) {
            this->non_empty_voxel(volume, current_idx, current_pos);
        }
    });

    volume.for_each_vertex_in_border(1, 1, 1, [&](auto x, auto y, auto z) {
        const glm::ivec3 current_idx(x, y, z);
        this->vertex(volume, current_idx);
    });

    std::transform(
            mesh_data.positions.begin(),
            mesh_data.positions.end(),
            std::back_inserter(mesh_data.brightnesses),
            [&](auto v) {
                assert(vertices_with_brightness.count(v) == 1);

                return vertices_with_brightness[v];
            });

    return mesh_data;
}

void MeshBuilder::non_empty_voxel(
        const Volume<Voxel> &volume,
        const glm::ivec3 current_idx,
        const glm::vec3 current_pos)
{
    for (auto neighbor : neighbor_dirs_with_face_vertex_positions) {
        const auto neighbor_idx = current_idx + neighbor.first;

        if (volume.at(neighbor_idx) == Voxel::empty) {
            auto &rel_positions = neighbor.second;
            const glm::vec3 normal(
                    neighbor.first.x, neighbor.first.y, neighbor.first.z);

            std::transform(
                    rel_positions.begin(),
                    rel_positions.end(),
                    std::back_inserter(mesh_data.positions),
                    [=](auto rel_pos) { return current_pos + rel_pos; });
            mesh_data.normals.insert(mesh_data.normals.end(), 6, normal);
        }
    }
}

void MeshBuilder::vertex(
        const Volume<Voxel> &volume, const glm::ivec3 current_idx)
{
    const glm::vec3 vertex(
            current_idx.x - 1, current_idx.y - 1, current_idx.z - 1);
    GLubyte nonempty_neighbor_voxel_count = 0;

    for (int dz = -1; dz <= 0; ++dz) {
        for (int dy = -1; dy <= 0; ++dy) {
            for (int dx = -1; dx <= 0; ++dx) {
                const glm::ivec3 neighbor_voxel_pos(
                        current_idx.x + dx,
                        current_idx.y + dy,
                        current_idx.z + dz);
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
