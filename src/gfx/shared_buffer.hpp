#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <ankerl/unordered_dense.h>
#include <vector>

struct ChunkBufferItem
{
    glm::ivec3 position;
    uint32_t color;
};
// storage buffer to store chunk data
// requests a handle to a section of the buffer
// and store command for command buffer
class SharedBuffer
{
public:
    SharedBuffer(unsigned long long preallocated_size, unsigned long long chunk_size);
    unsigned long long RequestNewChunkHandle();
    void FreeChunkHandle(unsigned long long handle, unsigned long long size);
    void UpdateChunk(unsigned long long handle, std::vector<ChunkBufferItem> &data);
    ~SharedBuffer();
    GLuint vao;

private:
    GLuint buffer;
    unsigned long long preallocated_size;
    unsigned long long chunk_size;
    std::vector<unsigned long long> free_chunks{};
};