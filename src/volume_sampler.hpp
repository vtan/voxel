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
            int border_size)
        const;
};

Volume<Voxel> VolumeSampler::sample_volume(
        const glm::ivec3 begin_coord,
        const glm::ivec3 end_coord,
        const int border_size)
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
        const double x = (begin_coord.x + (int)vx - border_size) / (double) x_size;
        const double y = (begin_coord.y + (int)vy - border_size) / (double) y_size;
        const double z = (begin_coord.z + (int)vz - border_size) / (double) z_size;

        const double w = 0.5 * (sin(x) * cos(z) + 1);

        if (y <= w) {
            volume.at(vx, vy, vz) = Voxel::solid;
        }
    });

    return volume;
}
