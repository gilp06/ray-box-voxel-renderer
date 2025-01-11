#pragma once

// voxel world class
// contains chunks
// chunks are defined in world/chunk.hpp

#include <glm/glm.hpp>
#include <unordered_map>
#include <world/chunk.hpp>
#include <variant>
#include <optional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// stores all chunks loaded in memory and manages chunk loading and unloading
// chunks are stored in a hashmap with their position as the key
// chunks are stored in a compressed format in memory
// when a chunk is deemed active it is decompressed and stored in an uncompressed format
// when a chunk is deemed inactive it is compressed and stored in a compressed format
// chunks are loaded and unloaded based on the player's position
// inactive chunks are still considered "loaded" but cannot be edited.

const int CHUNK_ACTIVE_DISTANCE = 4;
const int CHUNK_LOAD_DISTANCE = 8;

using ChunkVariantRef = std::variant<std::reference_wrapper<UncompressedChunk>, std::reference_wrapper<CompressedChunk>>;

class World
{
public:
    World() = default;

    ChunkVariantRef GetChunk(glm::ivec3 position);
    void NewChunk(glm::ivec3 position);
    void UnloadChunk(glm::ivec3 position);

    void ActivateChunk(glm::ivec3 position);
    void DeactivateChunk(glm::ivec3 position);

    void Update(glm::vec3 player_position);

private:
    std::unordered_map<glm::ivec3, UncompressedChunk> active_chunks;
    std::unordered_map<glm::ivec3, CompressedChunk> inactive_chunks;
};