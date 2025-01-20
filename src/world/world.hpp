#pragma once

// voxel world class
// contains chunks
// chunks are defined in world/chunk.hpp

#include <glm/glm.hpp>
#include <ankerl/unordered_dense.h>
// #include <utils/custom_hash.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <world/chunk.hpp>
#include <variant>
#include <optional>
#include <deque>

#include <unordered_set>
#include <unordered_map>
#include <memory>

#include <functional>

#include <GLFW/glfw3.h>
#include <FastNoiseLite.h>
#include <thread>
#include <mutex>
#include <future>

#include <ankerl/unordered_dense.h>
#include <utils/custom_hash.hpp>

constexpr int32_t WORLD_HEIGHT_IN_CHUNKS = 4;
constexpr int32_t CHUNK_DISTANCE = 32;

const int32_t LOD_RING_1 = 12;
const int32_t LOD_RING_2 = 20;
const int32_t LOD_RING_3 = 26;


// using ChunkVariantRef = std::variant<std::reference_wrapper<Chunk>, std::reference_wrapper<CompressedChunk>>;

class World
{
public:
    World();
    std::shared_ptr<Chunk> GenerateChunk(glm::ivec3 chunk_position);
    void RequestNewChunk(glm::ivec3 chunk_position);
    void Update(glm::vec3 player_position);
    bool ChunkLoaded(glm::ivec3 chunk_position);
    void UpdateNearbyChunks(glm::vec3 player_position);

    int EvaluateLoD(glm::ivec3 chunk_position);

    mutable std::mutex chunk_mutex;
    ankerl::unordered_dense::map<glm::ivec3, std::shared_ptr<Chunk>> loaded_chunks;
    ankerl::unordered_dense::map<glm::ivec3, std::future<std::shared_ptr<Chunk>>> generating_chunks;
    glm::vec3 player_position;
    glm::ivec3 player_chunk;
    FastNoiseLite noise;
};