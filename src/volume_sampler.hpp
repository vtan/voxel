#pragma once

#define GLM_FORCE_RADIANS

#include "volume.hpp"
#include "voxel.hpp"

#include <glm/glm.hpp>

class VolumeSampler
{
public:
    Volume<Voxel> sample_volume(
            glm::ivec3 min_coord,
            glm::ivec3 max_coord,
            size_t border_size)
        const;
};

Volume<Voxel> VolumeSampler::sample_volume(
        const glm::ivec3 begin_coord,
        const glm::ivec3 end_coord,
        const size_t border_size)
    const
{
    assert(begin_coord.x <= end_coord.x);
    assert(begin_coord.y <= end_coord.y);
    assert(begin_coord.z <= end_coord.z);

    const size_t x_size = end_coord.x - begin_coord.x;
    const size_t y_size = end_coord.y - begin_coord.y;
    const size_t z_size = end_coord.z - begin_coord.z;

    Volume<Voxel> volume(
            x_size + 2 * border_size,
            y_size + 2 * border_size,
            z_size + 2 * border_size,
            Voxel::empty);

    volume.for_each_voxel_in_border(0, 0, 0,
            [&](size_t vx, size_t vy, size_t vz) {
        const int x = begin_coord.x + vx - border_size;
        const int y = begin_coord.y + vy - border_size;
        const int z = begin_coord.z + vz - border_size;

        const bool floor = (y == 0 || y == end_coord.y - 1);
        const bool marker = (y == 1 || y == end_coord.y - 2)
                && x % x_size == 0
                && z % z_size == 0;
        if (floor || marker) {
            volume.at(vx, vy, vz) = Voxel::solid;
        }
    });

    return volume;
}
