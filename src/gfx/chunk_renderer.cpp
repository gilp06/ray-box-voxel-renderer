#include <gfx/chunk_renderer.hpp>
#include "chunk_renderer.hpp"

GPUChunk::GPUChunk(CompressedChunk &chunk)
{
    glGenBuffers(1, &vbo);
}

GPUChunk::GPUChunk(UncompressedChunk &chunk)
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void GPUChunk::Free()
{
    glDeleteBuffers(1, &vbo);
}

ChunkRenderer::ChunkRenderer()
{
    // binding vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // setup vertex attributes
    glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(ChunkBufferItem), (void *)offsetof(ChunkBufferItem, position));
    glVertexAttribPointer(1, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(ChunkBufferItem), (void *)offsetof(ChunkBufferItem, color));

    // enable vertex attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // setup shader
    shader = ShaderProgram("resources/shaders/chunk.vert.glsl", "resources/shaders/chunk.frag.glsl");
    shader.SetUniformBlock("ubo", 0);
}

ChunkRenderer::~ChunkRenderer()
{
    for (auto &chunk : chunks)
    {
        chunk.second.Free();
    }

    glDeleteVertexArrays(1, &vao);
}

void ChunkRenderer::Render()
{
    shader.Use();
    glBindVertexArray(vao);

    for (auto &chunk : chunks)
    {
        glBindBuffer(GL_ARRAY_BUFFER, chunk.second.vbo);
        glDrawArrays(GL_POINTS, 0, chunk.second.size);
    }
}
