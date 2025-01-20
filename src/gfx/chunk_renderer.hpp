#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <world/chunk.hpp>
#include <gfx/shader.hpp>

#include <glad/gl.h>
#include <world/world.hpp>
#include <deque>
#include <memory>
#include <map>

#include <gfx/cam.hpp>

#include <ankerl/unordered_dense.h>
#include <utils/custom_hash.hpp>
#include <gfx/shared_buffer.hpp>

// #define GLM_ENABLE_EXPERIMENTAL
// #include <glm/gtx/hash.hpp>


struct DrawArraysIndirectCommand
{
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint baseInstance;
};

class GPUChunk
{
public:
    GPUChunk(std::shared_ptr<SharedBuffer> buffer, glm::ivec3 position, std::shared_ptr<Chunk> chunk, int lod, std::mutex& lock);
    void UploadToBuffer();
    ~GPUChunk();

    bool onGPU = false;
    int lod = 1;

    glm::ivec3 position;
    // void UpdateMesh();
    std::shared_ptr<SharedBuffer> buffer;

    size_t block_count;
    unsigned long long chunk_handle;
    DrawArraysIndirectCommand indirect_command;
    std::vector<ChunkBufferItem> data;
};

class ChunkRenderer
{
public:
    ChunkRenderer(std::shared_ptr<World> world);
    ~ChunkRenderer();
    // void AddChunk(glm::ivec3 position, CompressedChunk &chunk);

    void Render(Camera &camera);
    void Update();
    std::shared_ptr<GPUChunk> LoadChunk(glm::ivec3 chunk_position, int lod);
    void RequestLoadChunk(glm::ivec3 chunk_position, int lod);

private:
    std::shared_ptr<World> world;
    ShaderProgram *shader = nullptr;

    // loaded chunks

    std::deque<glm::ivec3> chunks_to_load;
    ankerl::unordered_dense::set<glm::ivec3> chunks_to_load_set;

    mutable std::mutex gpu_chunk_mutex;
    ankerl::unordered_dense::segmented_map<glm::ivec3, std::shared_ptr<GPUChunk>> gpu_chunks;
    ankerl::unordered_dense::map<glm::ivec3, std::future<std::shared_ptr<GPUChunk>>> generating_gpu_chunks;

    std::shared_ptr<SharedBuffer> buffer;

    GLuint indirect_buffer;
    std::vector<DrawArraysIndirectCommand> indirect_commands;
};