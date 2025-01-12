#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <world/chunk.hpp>
#include <gfx/shader.hpp>

#include <glad/gl.h>

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
    GPUChunk() = default;
    GPUChunk(CompressedChunk &chunk);
    GPUChunk(UncompressedChunk &chunk);

    void Free();
    void Use();

    GLuint vbo;
    size_t size;
};

class ChunkRenderer
{
public:
    ChunkRenderer();
    ~ChunkRenderer();
    void AddChunk(glm::ivec3 position, CompressedChunk &chunk);
    void AddChunk(glm::ivec3 position, UncompressedChunk &chunk);
    void RemoveChunk(glm::ivec3 position);
    void Render();
    ShaderProgram* shader = nullptr;
    GLuint vao = 0;
    // loaded chunks
    std::unordered_map<glm::ivec3, GPUChunk> chunks;
};