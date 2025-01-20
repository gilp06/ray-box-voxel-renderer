#include "chunk_renderer.hpp"

#include <iostream>
#include <tracy/Tracy.hpp>

ChunkRenderer::ChunkRenderer(std::shared_ptr<World> world)
{
    this->world = world;
    // bbox size
    unsigned long long preallocated_size = 2 * (2 * CHUNK_DISTANCE + 1) * (2 * CHUNK_DISTANCE + 1) * WORLD_HEIGHT_IN_CHUNKS;
    buffer = std::make_shared<SharedBuffer>(preallocated_size, CHUNK_VOLUME);

    shader = new ShaderProgram("resources/shaders/chunk.vert.glsl", "resources/shaders/chunk.frag.glsl");
    shader->Use();
    shader->SetUniformBlock("ubo", 0);

    // world.SubscribeToChunkLoad([this](const glm::ivec3 &position, int lod)
    //                            { this->OnChunkLoad(position, lod); });
    // world.SubscribeToChunkUnload([this](const glm::ivec3 &position)
    //                              { this->OnChunkUnload(position); });
    // world.SubscribeToChunkUpdate([this](const glm::ivec3 &position, int lod)
    //                              { this->OnChunkUpdate(position, lod); });

    glCreateBuffers(1, &indirect_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer);

    glNamedBufferStorage(indirect_buffer, get_page_bytes(preallocated_size * sizeof(DrawArraysIndirectCommand), 65536), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_SPARSE_STORAGE_BIT_ARB);
    indirect_commands.reserve(preallocated_size);
}

void ChunkRenderer::Update()
{
    ZoneScoped;
    // look for changes in world
    std::lock_guard<std::mutex> lock(world->chunk_mutex);

    ZoneNamed(__tracy_1, "Gather Chunks To Load");
    for (auto &chunk : world->loaded_chunks)
    {
        if (chunks_to_load_set.find(chunk.first) == chunks_to_load_set.end() && gpu_chunks.find(chunk.first) == gpu_chunks.end())
        {
            int lod = world->EvaluateLoD(chunk.first);
            RequestLoadChunk(chunk.first, lod);
        }

        // case 2 where chunk is loaded but lod has changed
        if (gpu_chunks.find(chunk.first) != gpu_chunks.end())
        {
            // check if lod eq
            if (gpu_chunks[chunk.first]->lod != world->EvaluateLoD(chunk.first))
            {
                gpu_chunks.erase(chunk.first);
                int lod = world->EvaluateLoD(chunk.first);
                RequestLoadChunk(chunk.first, lod);
            }
        }
    }

    ZoneNamed(__tracy_2, "Gather Chunks To Unload");
    for (auto &chunk : gpu_chunks)
    {
        if (world->loaded_chunks.find(chunk.first) == world->loaded_chunks.end())
        {
            gpu_chunks.erase(chunk.first);
        }
    }
    std::lock_guard<std::mutex> gpu_lock(gpu_chunk_mutex);

    ZoneNamed(__tracy_3, "Gather Finished Chunks");
    for (auto futures = generating_gpu_chunks.begin(); futures != generating_gpu_chunks.end();)
    {
        auto &future_chunk = futures->second;
        if (future_chunk.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {

            std::shared_ptr<GPUChunk> chunk = future_chunk.get();
            gpu_chunks.try_emplace(futures->first, chunk);
            chunk->UploadToBuffer();
            futures = generating_gpu_chunks.erase(futures);
            // std::cout << "Chunk loaded at " << futures->first.x << " " << futures->first.y << " " << futures->first.z << std::endl;
        }
        else
        {
            futures++;
        }
    }
}

void ChunkRenderer::RequestLoadChunk(glm::ivec3 chunk_position, int lod)
{
    ZoneScoped;
    std::lock_guard<std::mutex> lock(gpu_chunk_mutex);

    {
        ZoneScoped;
        if (gpu_chunks.find(chunk_position) != gpu_chunks.end())
        {
            return;
        }

        if (generating_gpu_chunks.find(chunk_position) != generating_gpu_chunks.end())
        {
            return;
        }

        generating_gpu_chunks[chunk_position] = std::async(std::launch::async, &ChunkRenderer::LoadChunk, this, chunk_position, lod);
    }
}

std::shared_ptr<GPUChunk> ChunkRenderer::LoadChunk(glm::ivec3 chunk_position, int lod)
{
    ZoneScoped;
    return std::make_shared<GPUChunk>(buffer, chunk_position, world->loaded_chunks[chunk_position], lod, world->chunk_mutex);
}

void ChunkRenderer::Render(Camera &camera)
{
    ZoneScoped;
    std::lock_guard<std::mutex> lock(gpu_chunk_mutex);
    glBindVertexArray(buffer->vao);
    glm::mat4 frustum_space = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    std::vector<DrawArraysIndirectCommand> indirect_commands;
    for (auto &chunk : gpu_chunks)
    {
        if (chunk.second->onGPU)
        {
            indirect_commands.push_back(chunk.second->indirect_command);
        }
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

ChunkRenderer::~ChunkRenderer()
{
    delete shader;
}

GPUChunk::GPUChunk(std::shared_ptr<SharedBuffer> buffer, glm::ivec3 position, std::shared_ptr<Chunk> chunk, int lod, std::mutex &lock)
{
    ZoneScoped;
    this->position = position;
    this->lod = lod;

    // first pass, reduce chunk to CHUNK_SIZE/lod
    // in each cube, compress to most common block

    // compress chunk into a CHUNK_SIZE/lod cube
    int reduced_size = CHUNK_SIZE >> (lod - 1);
    int scale = 1 << (lod - 1);
    std::vector<uint8_t> reduced_chunk(reduced_size * reduced_size * reduced_size, 0);

    lock.lock();
    std::vector<uint8_t> blocks(chunk->GetBlocks(), chunk->GetBlocks() + CHUNK_VOLUME);
    lock.unlock();

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
        reduced_chunk = std::vector<uint8_t>(blocks);
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

    this->buffer = buffer;
}

void GPUChunk::UploadToBuffer()
{
    onGPU = true;
    block_count = data.size();
    // std::cout << "Block count: " << block_count << std::endl;
    chunk_handle = buffer->RequestNewChunkHandle();
    buffer->UpdateChunk(chunk_handle, data);

    indirect_command.first = chunk_handle * CHUNK_VOLUME;
    indirect_command.count = block_count;
    indirect_command.instanceCount = 1;
    indirect_command.baseInstance = 0;
}

GPUChunk::~GPUChunk()
{
    // std::cout << "Free chunk handle" << std::endl;
    if (chunk_handle != -1)
        buffer->FreeChunkHandle(chunk_handle, block_count);
    // std::cout << "Freed chunk handle" << std::endl;
}