#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <world/chunk.hpp>
#include <gfx/shader.hpp>

struct ChunkBufferItem
{
    glm::u8vec3 position;
    uint32_t color;
};

class ChunkBuffer
{
public:
    ChunkBuffer(CompressedChunk &chunk);
    ChunkBuffer(UncompressedChunk &chunk);
    std::vector<ChunkBufferItem> buffer;
};

class ChunkRenderer
{
public:
    ChunkRenderer();
    void AddChunk(glm::ivec3 position, CompressedChunk &chunk);
    void AddChunk(glm::ivec3 position, UncompressedChunk &chunk);
    void RemoveChunk(glm::ivec3 position);
    ShaderProgram shader;
    // loaded chunks
    std::unordered_map<glm::ivec3, ChunkBuffer> chunks;
};