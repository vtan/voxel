#pragma once

#define GLM_FORCE_RADIANS

#include "heightmap.hpp"
#include "volume.hpp"
#include "voxel.hpp"

#include <glm/glm.hpp>

Heightmap sample_heightmap(glm::ivec3 begin_coord, glm::ivec3 end_coord,
        int border_size)
{
    assert(begin_coord.x <= end_coord.x);
    assert(begin_coord.y <= end_coord.y);
    assert(begin_coord.z <= end_coord.z);

    const size_t x_size = end_coord.x - begin_coord.x;
    const size_t y_size = end_coord.y - begin_coord.y;
    const size_t z_size = end_coord.z - begin_coord.z;

    Heightmap heightmap(x_size + 2 * border_size, z_size + 2 * border_size);
    for (size_t vz = 0; vz < heightmap.z_size(); ++vz) {
        for (size_t vx = 0; vx < heightmap.x_size(); ++vx) {
            const double x = (begin_coord.x + (int)vx - border_size) / (double) x_size;
            const double z = (begin_coord.z + (int)vz - border_size) / (double) z_size;

            const double y = 0.5 * (sin(x) * cos(z) + 1);
            const uint8_t height = y_size * y;
            heightmap.at(vx, vz) = height;

            assert(height >= begin_coord.y);
            assert(height < end_coord.y);
        }
    }
    return heightmap;
}

Volume<Voxel> volume_from_heightmap(const Heightmap &heightmap, size_t y_size,
        size_t border_size)
{
    const size_t x_size = heightmap.x_size();
    const size_t z_size = heightmap.z_size();

    Volume<Voxel> volume(x_size, y_size + 2 * border_size, z_size,
            Voxel::empty);
    for (size_t z = 0; z < z_size; ++z) {
        for (size_t x = 0; x < x_size; ++x) {
            const uint8_t height = heightmap.at(x, z);
            for (uint8_t y = 0; y <= border_size + height; ++y) {
                volume.at(x, y, z) = Voxel::solid;
            }
        }
    }
    return volume;
}
