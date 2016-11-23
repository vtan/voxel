#pragma once

#include "chunk_volume_repository.hpp"
#include "mesh.hpp"
#include "mesh_builder.hpp"

#include <unordered_map>

class ChunkMeshRepository
{
public:
    ChunkMeshRepository(ChunkVolumeRepository& cvr)
        : chunk_volume_repository(cvr) {}

    template <typename F>
    void with(ChunkId, F);
private:
    ChunkVolumeRepository& chunk_volume_repository;
    std::unordered_map<ChunkId, Mesh> meshes;
    MeshBuilder mesh_builder;

    Mesh& get_or_build(ChunkId);
};

template <typename F>
void ChunkMeshRepository::with(const ChunkId chunk_id, const F f)
{
    f(get_or_build(chunk_id));
}

Mesh& ChunkMeshRepository::get_or_build(const ChunkId chunk_id)
{
    auto found = meshes.find(chunk_id);
    if (found != meshes.end()) {
        return found->second;
    } else {
        Mesh mesh;
        chunk_volume_repository.with(chunk_id, [&] (auto volume) {
            mesh.build_vao(mesh_builder.build(volume));
        });

        auto inserted = meshes.insert({chunk_id, mesh});
        return inserted.first->second;
    }
}
