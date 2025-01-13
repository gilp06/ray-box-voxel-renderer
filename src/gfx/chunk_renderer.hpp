#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <world/chunk.hpp>
#include <gfx/shader.hpp>

#include <glad/gl.h>
#include <world/world.hpp>
#include <deque>
#include <map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct ChunkBufferItem
{
    glm::uvec3 position;
    uint32_t color;
};

class GPUChunk
{
public:
    // GPUChunk(CompressedChunk &chunk);
    GPUChunk(glm::ivec3 position, World& w);
    ~GPUChunk();
    size_t size;

    GLuint vao = 0;
    GLuint vbo = 0;
};

class ChunkRenderer
{
public:
    ChunkRenderer(World &world);
    ~ChunkRenderer();
    // void AddChunk(glm::ivec3 position, CompressedChunk &chunk);

    void AddChunk(glm::ivec3 position);
    void RemoveChunk(glm::ivec3 position);
    void Render();
    void Update();

    void OnChunkLoad(const glm::ivec3 &position);
    void OnChunkUnload(const glm::ivec3 &position);
    void OnChunkUpdate(const glm::ivec3 &position);

private:
    World &world;
    ShaderProgram *shader = nullptr;

    // loaded chunks
    std::unordered_map<glm::ivec3, GPUChunk> chunks;
    std::deque<glm::ivec3> chunks_to_gen_mesh;
};