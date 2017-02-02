#pragma once

#include "chunk.hpp"
#include "log.hpp"
#include "volume.hpp"
#include "voxel.hpp"

#include <unordered_map>

class ChunkVolumeRepository
{
public:
    typedef std::function<Volume<Voxel>(glm::ivec3, glm::ivec3, int)> VolumeSampler;

    ChunkVolumeRepository(VolumeSampler vs, int bs)
        : volume_sampler(vs)
        , border_size(bs) {}

    template <typename F>
    void with(ChunkId, F);
private:
    const VolumeSampler volume_sampler;
    const int border_size;

    std::unordered_map<ChunkId, Volume<Voxel>> volumes;

    Volume<Voxel>& get_or_sample(ChunkId);
};

template <typename F>
void ChunkVolumeRepository::with(const ChunkId chunk_id, const F f)
{
    f(get_or_sample(chunk_id));
}

Volume<Voxel>& ChunkVolumeRepository::get_or_sample(const ChunkId chunk_id)
{
    auto found = volumes.find(chunk_id);
    if (found != volumes.end()) {
        return found->second;
    } else {
        Log::debug("Sampling volume at " << chunk_id);
        const Volume<Voxel> volume = volume_sampler(
                Chunks::begin_coord(chunk_id),
                Chunks::end_coord(chunk_id),
                border_size);
        auto inserted = volumes.insert({chunk_id, volume});
        return inserted.first->second;
    }
}
