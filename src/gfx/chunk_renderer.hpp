#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <world/chunk.hpp>
#include <gfx/shader.hpp>

#include <glad/gl.h>

struct ChunkBufferItem
{
    glm::u8vec3 position;
    uint32_t color;
};

class GPUChunk
{
public:
    GPUChunk() = default;
    GPUChunk(CompressedChunk &chunk);
    GPUChunk(UncompressedChunk &chunk);

    void Free();
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
    ShaderProgram shader;
    GLuint vao;
    // loaded chunks
    std::unordered_map<glm::ivec3, GPUChunk> chunks;
};