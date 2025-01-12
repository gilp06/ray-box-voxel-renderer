#include <gfx/chunk_renderer.hpp>
#include <iostream>

GPUChunk::GPUChunk(CompressedChunk &chunk) : GPUChunk(chunk.Decompress())
{
}

GPUChunk::GPUChunk(UncompressedChunk &chunk)
{
    std::vector<ChunkBufferItem> buffer;
    for (size_t i = 0; i < 4096; i++)
    {
        BlockData block = BlockManager::GetBlockData(chunk.GetBlocks()[i]);
        if (block.type == BlockType::Empty)
            continue;

        ChunkBufferItem item;
        item.position = glm::u8vec3(deinterleaveBits(i, 2), deinterleaveBits(i, 1), deinterleaveBits(i, 0));
        item.color = block.color;

        buffer.push_back(item);
    }

    size = buffer.size();
    if (size == 0)
    {
        return;
    }

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(ChunkBufferItem), buffer.data(), GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, sizeof(ChunkBufferItem), (void *)offsetof(ChunkBufferItem, position));
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(ChunkBufferItem), (void *)offsetof(ChunkBufferItem, color));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GPUChunk::Free()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);
}

void GPUChunk::Use()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

ChunkRenderer::ChunkRenderer()
{
    // binding vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // setup vertex attributes
    // glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, sizeof(ChunkBufferItem), (void *)offsetof(ChunkBufferItem, position));
    // glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(ChunkBufferItem), (void *)offsetof(ChunkBufferItem, color));

    // setup shader
    shader = new ShaderProgram("resources/shaders/chunk.vert.glsl", "resources/shaders/chunk.frag.glsl");
    shader->SetUniformBlock("ubo", 0);
}

ChunkRenderer::~ChunkRenderer()
{
    for (auto &chunk : chunks)
    {
        chunk.second.Free();
    }

    glDeleteVertexArrays(1, &vao);
    delete shader;
}

void ChunkRenderer::AddChunk(glm::ivec3 position, CompressedChunk &chunk)
{
    auto outcome = chunks.try_emplace(position, chunk);
}

void ChunkRenderer::AddChunk(glm::ivec3 position, UncompressedChunk &chunk)
{
    auto outcome = chunks.try_emplace(position, chunk);
    // print whether or not the chunk was added
}

void ChunkRenderer::RemoveChunk(glm::ivec3 position)
{
    auto it = chunks.find(position);
    if (it != chunks.end())
    {
        it->second.Free();
        chunks.erase(it);
    }
}

void ChunkRenderer::Render()
{
    glBindVertexArray(vao);

    for (auto &chunk : chunks)
    {
        if (chunk.second.size == 0)
            continue;

        shader->Use();
        shader->SetUniformIVec3("chunk_pos", glm::vec3(chunk.first));

        chunk.second.Use();
        glDrawArrays(GL_POINTS, 0, chunk.second.size);
    }
}
