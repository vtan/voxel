#pragma once

#define GLM_FORCE_RADIANS

#include "volume.hpp"
#include "voxel.hpp"

#include <glm/glm.hpp>

class VolumeSampler
{
public:
    Volume<Voxel> sample_volume(glm::ivec3 min_coord, glm::ivec3 max_coord)
        const;
};

Volume<Voxel> VolumeSampler::sample_volume(
        const glm::ivec3 begin_coord, const glm::ivec3 end_coord) const
{
    assert(begin_coord.x <= end_coord.x);
    assert(begin_coord.y <= end_coord.y);
    assert(begin_coord.z <= end_coord.z);

    const size_t x_size = end_coord.x - begin_coord.x;
    const size_t y_size = end_coord.y - begin_coord.y;
    const size_t z_size = end_coord.z - begin_coord.z;

    Volume<Voxel> volume(x_size, y_size, z_size, Voxel::empty);

    volume.for_each_voxel_in_border(1, 1, 1, [&](size_t x, size_t y, size_t z) {
        const bool floor = (y == 1 || y == y_size - 2);
        const bool marker = (x <= 1 || x >= x_size - 2)
                && (z <= 1 || z >= z_size - 2)
                && (y == 2 || y == y_size - 3);
        if (floor || marker) {
            volume.at(x, y, z) = Voxel::solid;
        }
    });

    return volume;
}
