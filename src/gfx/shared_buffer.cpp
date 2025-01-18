#include "shared_buffer.hpp"

#include <iostream>

SharedBuffer::SharedBuffer(unsigned long long preallocated_size, unsigned long long chunk_size)
{
    this->preallocated_size = preallocated_size;
    this->chunk_size = chunk_size;
    unsigned long long full_size = preallocated_size * chunk_size * sizeof(ChunkBufferItem);
    std::cout << "Full size: " << full_size << std::endl;

    glCreateBuffers(1, &buffer);
    glNamedBufferData(buffer, full_size, nullptr, GL_DYNAMIC_DRAW);
    glCreateVertexArrays(1, &vao);

    // configure vao
    glVertexArrayVertexBuffer(vao, 0, buffer, 0, sizeof(ChunkBufferItem));

    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);

    glVertexArrayAttribIFormat(vao, 0, 3, GL_INT, offsetof(ChunkBufferItem, position));
    glVertexArrayAttribIFormat(vao, 1, 1, GL_UNSIGNED_INT, offsetof(ChunkBufferItem, color));

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);

    glBindVertexArray(0);

    for(long i = preallocated_size-1; i >= 0; i--)
    {
        free_chunks.push_back(i);
    }
}

unsigned long long SharedBuffer::RequestNewChunkHandle()
{
    // get the first free chunk
    if(free_chunks.empty())
    {
        std::cerr << "No more free chunks" << std::endl;
        return -1;
    }
    auto chunk = free_chunks.back();
    free_chunks.pop_back();
    return chunk;
}

void SharedBuffer::FreeChunkHandle(unsigned long long handle)
{
    // std::cout << "Free chunk " << handle << "/" << this->preallocated_size << std::endl;
    free_chunks.emplace_back(handle);
    // std::cout << "Freed chunk " << handle << std::endl;
}

SharedBuffer::~SharedBuffer()
{
    glDeleteBuffers(1, &buffer);
    glDeleteVertexArrays(1, &vao);
}

void SharedBuffer::UpdateChunk(unsigned long long handle, std::vector<ChunkBufferItem> &data)
{
    auto offset = handle * this->chunk_size * sizeof(ChunkBufferItem);
    glNamedBufferSubData(buffer, offset, data.size() * sizeof(ChunkBufferItem), data.data());
}