#include "chunk_renderer.hpp"

#include <iostream>
#include <tracy/Tracy.hpp>

GPUChunk::GPUChunk(std::shared_ptr<SharedBuffer> buffer, glm::ivec3 position, std::shared_ptr<Chunk> chunk)
{
    this->position = position;

    std::vector<ChunkBufferItem> data;
    for (int i = 0; i < CHUNK_VOLUME; i++)
    {
        if (chunk->GetBlocks()[i] == BlockManager::GetBlockIndex("air"))
        {
            continue;
        }
        glm::ivec3 world_pos = (position * (int)CHUNK_SIZE) + glm::ivec3(deinterleaveBits(i, 2), deinterleaveBits(i, 1), deinterleaveBits(i, 0));

        // inject lod data into color
        // random pick one through 4

        int lod = rand() % 4 + 1;
        data.push_back({world_pos, BlockManager::GetBlockData(chunk->GetBlocks()[i]).color | lod});

        // data.push_back({world_pos, BlockManager::GetBlockData();
    }

    block_count = data.size();
    // std::cout << "Block count: " << block_count << std::endl;
    chunk_handle = buffer->RequestNewChunkHandle();
    buffer->UpdateChunk(chunk_handle, data);

    indirect_command.first = chunk_handle * CHUNK_VOLUME;
    indirect_command.count = block_count;
    indirect_command.instanceCount = 1;
    indirect_command.baseInstance = 0;

    this->buffer = buffer;
}

GPUChunk::~GPUChunk()
{
    // std::cout << "Free chunk handle" << std::endl;
    buffer->FreeChunkHandle(chunk_handle, block_count);
    // std::cout << "Freed chunk handle" << std::endl;
}  

ChunkRenderer::ChunkRenderer(World &world) : world(world)
{

    // bbox size
    unsigned long long preallocated_size = 2 * (2 * CHUNK_DISTANCE + 1) * (2 * CHUNK_DISTANCE + 1) * WORLD_HEIGHT_IN_CHUNKS;
    buffer = std::make_shared<SharedBuffer>(preallocated_size, CHUNK_VOLUME);

    shader = new ShaderProgram("resources/shaders/chunk.vert.glsl", "resources/shaders/chunk.frag.glsl");
    shader->Use();
    shader->SetUniformBlock("ubo", 0);

    world.SubscribeToChunkLoad([this](const glm::ivec3 &position)
                               { this->OnChunkLoad(position); });
    world.SubscribeToChunkUnload([this](const glm::ivec3 &position)
                                 { this->OnChunkUnload(position); });
    world.SubscribeToChunkUpdate([this](const glm::ivec3 &position)
                                 { this->OnChunkUpdate(position); });

    glCreateBuffers(1, &indirect_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer);
    
    glNamedBufferStorage(indirect_buffer, get_page_bytes(preallocated_size * sizeof(DrawArraysIndirectCommand), 65536), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_SPARSE_STORAGE_BIT_ARB);
    indirect_commands.reserve(preallocated_size);
}

void ChunkRenderer::AddChunk(glm::ivec3 position)
{
    auto chunk = world.GetChunk(position);
    if (chunk == nullptr)
    {
        return;
    }
    auto gpu_chunk = std::make_shared<GPUChunk>(buffer, position, chunk);
    chunks.try_emplace(position, gpu_chunk);
}

void ChunkRenderer::RemoveChunk(glm::ivec3 position)
{
    // std::cout << "Remove chunk" << std::endl;
    chunks.erase(position);
    // std::cout << "Removed chunk" << std::endl;
}

void ChunkRenderer::Render(Camera &camera)
{
    ZoneScoped;
    glBindVertexArray(buffer->vao);
    glm::mat4 frustum_space = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    std::vector<DrawArraysIndirectCommand> indirect_commands;
    for (auto &chunk : chunks)
    {
        // if (within_frustum(glm::vec3(chunk.second->position), frustum_space))
        // {
        //     indirect_commands.push_back(chunk.second->indirect_command);
        // }
        indirect_commands.push_back(chunk.second->indirect_command);
    }

    // std::cout << "Try draw " << indirect_commands.size() << " chunks" << std::endl;
    if (indirect_commands.size() == 0)
    {
        indirect_commands.clear();
        return;
    }

    // std::cout << "Draw " << indirect_commands.size() << " chunks" << std::endl;

    // gen data for indirect draw
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer);
    // buffer page size is 65536, commit lowest number of pages
    size_t pages_needed = get_page_bytes(indirect_commands.size() * sizeof(ChunkBufferItem), 65536);
    glNamedBufferPageCommitmentARB(indirect_buffer, 0, pages_needed, GL_TRUE);
    glNamedBufferSubData(indirect_buffer, 0, indirect_commands.size() * sizeof(DrawArraysIndirectCommand), indirect_commands.data());
    glMultiDrawArraysIndirect(GL_POINTS, 0, indirect_commands.size(), 0);
    // remove pages
    glNamedBufferPageCommitmentARB(indirect_buffer, 0, pages_needed, GL_FALSE);

    indirect_commands.clear();
}

void ChunkRenderer::Update()
{
}

void ChunkRenderer::OnChunkLoad(const glm::ivec3 &position)
{
    AddChunk(position);
}

void ChunkRenderer::OnChunkUnload(const glm::ivec3 &position)
{
    RemoveChunk(position);
}

void ChunkRenderer::OnChunkUpdate(const glm::ivec3 &position)
{
    RemoveChunk(position);
    AddChunk(position);
}

ChunkRenderer::~ChunkRenderer()
{
    delete shader;
}