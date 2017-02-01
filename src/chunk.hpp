#pragma once

#define GLM_FORCE_RADIANS

#include <functional>
#include <ostream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct ChunkId
{
    int x;
    int z;
};

bool operator==(const ChunkId lhs, const ChunkId rhs)
{
    return lhs.x == rhs.x && lhs.z == rhs.z;
}

std::ostream& operator<<(std::ostream& os, const ChunkId chunk_id)
{
    os << '{' << chunk_id.x << ',' << chunk_id.z << '}';
    return os;
}

namespace std
{
template<> struct hash<ChunkId>
{
    size_t operator()(const ChunkId chunk_id) const
    {
        const uint64_t ux = chunk_id.x;
        const uint64_t uz = chunk_id.z;
        return std::hash<uint64_t>{}(ux << 32 | uz);
    }
};
}

namespace Chunks
{

constexpr int x_size = 64;
constexpr int z_size = 64;

constexpr int y_begin = 0;
constexpr int y_end = 64;

glm::ivec3 begin_coord(const ChunkId chunk_id)
{
    return {chunk_id.x * x_size, y_begin, chunk_id.z * z_size};
}

glm::ivec3 end_coord(const ChunkId chunk_id)
{
    return {(chunk_id.x + 1) * x_size, y_end, (chunk_id.z + 1) * z_size};
}

ChunkId chunk_at(const glm::vec3 p)
{
    const auto x = (int) std::floor(p.x / x_size);
    const auto z = (int) std::floor(p.z / z_size);
    return { x, z };
}

glm::mat4 calc_translation(const ChunkId chunk_id)
{
    return glm::translate(glm::vec3(
                x_size * chunk_id.x, 0.f, z_size * chunk_id.z));
}

}
