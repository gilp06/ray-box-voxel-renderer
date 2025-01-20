#include "chunk_renderer.hpp"

#include <iostream>
#include <tracy/Tracy.hpp>

GPUChunk::GPUChunk(std::shared_ptr<SharedBuffer> buffer, glm::ivec3 position, std::shared_ptr<Chunk> chunk, int lod)
{
    ZoneScoped;
    this->position = position;

    // first pass, reduce chunk to CHUNK_SIZE/lod
    // in each cube, compress to most common block

    // compress chunk into a CHUNK_SIZE/lod cube
    int reduced_size = CHUNK_SIZE >> (lod - 1);
    int scale = 1 << (lod - 1);
    std::vector<uint8_t> reduced_chunk(reduced_size * reduced_size * reduced_size, 0);
    const uint8_t *blocks = chunk->GetBlocks();

    // check if chunk is empty
    bool empty = true;
    for (int i = 0; i < CHUNK_VOLUME; i++)
    {
        if (blocks[i] != BlockManager::GetBlockIndex("air"))
        {
            empty = false;
            break;
        }
    }

    if (empty)
    {
        block_count = 0;
        chunk_handle = -1;

        indirect_command.first = chunk_handle * CHUNK_VOLUME;
        indirect_command.count = block_count;
        indirect_command.instanceCount = 1;
        indirect_command.baseInstance = 0;

        this->buffer = buffer;
        return;
    }

    if (lod == 1)
    {
        // if one skip the lod generation
        reduced_chunk = std::vector<uint8_t>(blocks, blocks + CHUNK_VOLUME);
    }
    else
    {
        for (int x = 0; x < reduced_size; x++)
        {
            for (int y = 0; y < reduced_size; y++)
            {
                for (int z = 0; z < reduced_size; z++)
                {
                    ankerl::unordered_dense::map<uint8_t, int> block_count;
                    for (int i = 0; i < scale; i++)
                    {
                        for (int j = 0; j < scale; j++)
                        {
                            for (int k = 0; k < scale; k++)
                            {
                                int index = interleaveBits(x * scale + i, y * scale + j, z * scale + k);

                                if (index < 0 || index >= CHUNK_VOLUME)
                                {
                                    std::cout << "Index out of bounds: " << index << std::endl;
                                }
                                uint8_t block = blocks[index];
                                // check if air block and skip
                                if (block == BlockManager::GetBlockIndex("air"))
                                {
                                    continue;
                                }
                                block_count[block]++;
                            }
                        }
                    }
                    uint8_t most_common_block = 0;
                    int most_common_block_count = 0;

                    for (auto &pair : block_count)
                    {
                        if (pair.second > most_common_block_count)
                        {
                            most_common_block = pair.first;
                            most_common_block_count = pair.second;
                        }
                    }
                    // if empty chunk, set to air
                    if (most_common_block_count == 0)
                    {
                        most_common_block = BlockManager::GetBlockIndex("air");
                    }

                    glm::ivec3 reduced_position(x, y, z);
                    reduced_chunk[interleaveBits(x, y, z)] = most_common_block;
                }
            }
        }
    }

    // second pass, pass only transparent blocks

    std::vector<ChunkBufferItem> data;
    for (int i = 0; i < reduced_chunk.size(); i++)
    {
        if (reduced_chunk[i] == BlockManager::GetBlockIndex("air"))
        {
            continue;
        }

        bool visible = false;
        constexpr glm::ivec3 offsets[] = {
            glm::ivec3(1, 0, 0),
            glm::ivec3(-1, 0, 0),
            glm::ivec3(0, 1, 0),
            glm::ivec3(0, -1, 0),
            glm::ivec3(0, 0, 1),
            glm::ivec3(0, 0, -1)};

        glm::ivec3 block_pos_in_chunk = glm::ivec3(deinterleaveBits(i, 2), deinterleaveBits(i, 1), deinterleaveBits(i, 0));

        for (int i = 0; i < 6; i++)
        {
            glm::ivec3 offset = offsets[i];
            int index = interleaveBits(block_pos_in_chunk.x + offset.x, block_pos_in_chunk.y + offset.y, block_pos_in_chunk.z + offset.z);
            if (index < 0 || index >= reduced_chunk.size())
            {
                continue;
            }
            if (reduced_chunk[index] == BlockManager::GetBlockIndex("air"))
            {
                visible = true;
                break;
            }
        }

        if (!visible)
        {
            continue;
        }

        glm::ivec3 world_pos = (position * (int)CHUNK_SIZE) + glm::ivec3(deinterleaveBits(i, 2), deinterleaveBits(i, 1), deinterleaveBits(i, 0)) * scale;
        data.push_back({world_pos, BlockManager::GetBlockData(reduced_chunk[i]).color | scale});
    }

    // std::vector<ChunkBufferItem> data;
    // for (int i = 0; i < CHUNK_VOLUME; i++)
    // {
    //     if (chunk->GetBlocks()[i] == BlockManager::GetBlockIndex("air"))
    //     {
    //         continue;
    //     }
    //     glm::ivec3 world_pos = (position * (int)CHUNK_SIZE) + glm::ivec3(deinterleaveBits(i, 2), deinterleaveBits(i, 1), deinterleaveBits(i, 0));

    //     // inject lod data into color
    //     // random pick one through 4

    //     data.push_back({world_pos, BlockManager::GetBlockData(chunk->GetBlocks()[i]).color | 1});

    //     // data.push_back({world_pos, BlockManager::GetBlockData();
    // }

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
    if(chunk_handle != -1)
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

    world.SubscribeToChunkLoad([this](const glm::ivec3 &position, int lod)
                               { this->OnChunkLoad(position, lod); });
    world.SubscribeToChunkUnload([this](const glm::ivec3 &position)
                                 { this->OnChunkUnload(position); });
    world.SubscribeToChunkUpdate([this](const glm::ivec3 &position, int lod)
                                 { this->OnChunkUpdate(position, lod); });

    glCreateBuffers(1, &indirect_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer);

    glNamedBufferStorage(indirect_buffer, get_page_bytes(preallocated_size * sizeof(DrawArraysIndirectCommand), 65536), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_SPARSE_STORAGE_BIT_ARB);
    indirect_commands.reserve(preallocated_size);
}

void ChunkRenderer::AddChunk(glm::ivec3 position, int lod)
{
    auto chunk = world.GetChunk(position);
    if (chunk == nullptr)
    {
        return;
    }
    auto gpu_chunk = std::make_shared<GPUChunk>(buffer, position, chunk, lod);
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

void ChunkRenderer::OnChunkLoad(const glm::ivec3 &position, int lod)
{
    AddChunk(position, lod);
}

void ChunkRenderer::OnChunkUnload(const glm::ivec3 &position)
{
    RemoveChunk(position);
}

void ChunkRenderer::OnChunkUpdate(const glm::ivec3 &position, int lod)
{
    RemoveChunk(position);
    AddChunk(position, lod);
}

ChunkRenderer::~ChunkRenderer()
{
    delete shader;
}