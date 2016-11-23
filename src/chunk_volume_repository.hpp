#pragma once

#include "chunk.hpp"
#include "volume.hpp"
#include "volume_sampler.hpp"
#include "voxel.hpp"

#include <unordered_map>

class ChunkVolumeRepository
{
public:
    template <typename F>
    void with(ChunkId, F);
private:
    std::unordered_map<ChunkId, Volume<Voxel>> volumes;
    VolumeSampler volume_sampler;

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
        const Volume<Voxel> volume = volume_sampler.sample_volume(
                Chunks::begin_coord(chunk_id),
                Chunks::end_coord(chunk_id));
        auto inserted = volumes.insert({chunk_id, volume});
        return inserted.first->second;
    }
}
