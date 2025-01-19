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
    GPUChunk(std::shared_ptr<SharedBuffer> buffer, glm::ivec3 position, std::shared_ptr<Chunk> chunk, int lod);
    ~GPUChunk();
    glm::ivec3 position;
    // void UpdateMesh();
    std::shared_ptr<SharedBuffer> buffer;

    size_t block_count;
    unsigned long long chunk_handle;
    DrawArraysIndirectCommand indirect_command;
};

class ChunkRenderer
{
public:
    ChunkRenderer(World &world);
    ~ChunkRenderer();
    // void AddChunk(glm::ivec3 position, CompressedChunk &chunk);

    void AddChunk(glm::ivec3 position, int lod = 1);
    void RemoveChunk(glm::ivec3 position);
    void Render(Camera &camera);
    void Update();

    void OnChunkLoad(const glm::ivec3 &position, int lod = 1);
    void OnChunkUnload(const glm::ivec3 &position);
    void OnChunkUpdate(const glm::ivec3 &position, int lod = 1);

private:
    World &world;
    ShaderProgram *shader = nullptr;

    // loaded chunks
    ankerl::unordered_dense::segmented_map<glm::ivec3, std::shared_ptr<GPUChunk>> chunks;
    std::deque<glm::ivec3> chunks_to_gen_mesh;

    std::shared_ptr<SharedBuffer> buffer;

    GLuint indirect_buffer;
    std::vector<DrawArraysIndirectCommand> indirect_commands;
};