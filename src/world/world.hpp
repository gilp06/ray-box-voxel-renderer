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

#include <ankerl/unordered_dense.h>
#include <utils/custom_hash.hpp>

constexpr int32_t WORLD_HEIGHT_IN_CHUNKS = 4;
constexpr int32_t CHUNK_DISTANCE = 4;

// using ChunkVariantRef = std::variant<std::reference_wrapper<Chunk>, std::reference_wrapper<CompressedChunk>>;

class World
{
public:
    using ChunkCallback = std::function<void(const glm::ivec3 &)>;
    World();

    std::shared_ptr<Chunk> GetChunk(glm::ivec3 position);
    std::shared_ptr<Chunk> NewChunk(glm::ivec3 position);
    void UnloadChunk(glm::ivec3 position);
    bool ChunkLoaded(glm::ivec3 position);

    // void ActivateChunk(glm::ivec3 position);
    // void DeactivateChunk(glm::ivec3 position);

    void Update(glm::vec3 player_position);
    void UpdateAdjacentChunks(glm::ivec3 position);

    void SubscribeToChunkLoad(ChunkCallback callback);
    void SubscribeToChunkUnload(ChunkCallback callback);
    void SubscribeToChunkUpdate(ChunkCallback callback);

    void Input(GLFWwindow *window);


private:
    ankerl::unordered_dense::segmented_map<glm::ivec3, std::shared_ptr<Chunk>> active_chunks;
    std::deque<glm::ivec3> chunks_to_load;
    std::deque<glm::ivec3> chunks_to_unload;
    std::unordered_set<glm::ivec3> chunks_to_load_set;
    std::unordered_set<glm::ivec3> chunks_to_unload_set;


    std::vector<ChunkCallback> chunk_load_callbacks;
    std::vector<ChunkCallback> chunk_unload_callbacks;
    std::vector<ChunkCallback> chunk_update_callbacks;

    void AddChunkToLoad(const glm::ivec3 &position);
    void AddChunkToUnload(const glm::ivec3 &position);
    bool FetchNextChunkToLoad(glm::ivec3& chunk_pos);
    bool FetchNextChunkToUnload(glm::ivec3& chunk_pos);

    void NotifyChunkLoad(const glm::ivec3 &position);
    void NotifyChunkUnload(const glm::ivec3 &position);
    void NotifyChunkUpdate(const glm::ivec3 &position);
    // std::unordered_map<glm::ivec3, CompressedChunk> inactive_chunks;
};