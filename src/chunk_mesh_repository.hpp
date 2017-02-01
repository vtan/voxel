#pragma once

#include "chunk_volume_repository.hpp"
#include "mesh.hpp"
#include "mesh_builder.hpp"

#include <unordered_map>

struct TimestampedMesh
{
    Mesh mesh;
    std::time_t last_access;
};

class ChunkMeshRepository
{
public:
    ChunkMeshRepository(ChunkVolumeRepository& cvr, size_t cap)
        : chunk_volume_repository(cvr), capacity(cap) {}

    template <typename F>
    void with(ChunkId, F);
private:
    ChunkVolumeRepository& chunk_volume_repository;
    const size_t capacity;

    std::unordered_map<ChunkId, TimestampedMesh> meshes;
    MeshBuilder mesh_builder;

    Mesh& get_or_build(ChunkId);
    Mesh build(ChunkId);
    void remove_oldest_accessed();
    Mesh& store(ChunkId, Mesh&&);
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
        found->second.last_access = std::time(nullptr);
        return found->second.mesh;
    } else {
        Mesh mesh = build(chunk_id);
        if (meshes.size() == capacity) {
            remove_oldest_accessed();
        }
        return store(chunk_id, std::move(mesh));
    }
}

Mesh ChunkMeshRepository::build(ChunkId chunk_id)
{
    Log::debug("Building mesh at " << chunk_id);
    Mesh mesh;
    chunk_volume_repository.with(chunk_id, [&](auto volume) {
        mesh.build_vao(mesh_builder.build(volume));
    });
    return mesh;
}

void ChunkMeshRepository::remove_oldest_accessed()
{
    auto oldest_accessed = std::min_element(meshes.begin(), meshes.end(),
            [](auto& a, auto& b) {
        return a.second.last_access < b.second.last_access;
    });
    Log::debug("Removing mesh at " << oldest_accessed->first);
    meshes.erase(oldest_accessed);
}

Mesh& ChunkMeshRepository::store(ChunkId chunk_id, Mesh&& mesh)
{
    TimestampedMesh timestamped_mesh { mesh, std::time(nullptr) };
    auto inserted = meshes.insert({ chunk_id, timestamped_mesh });
    return inserted.first->second.mesh;
}
