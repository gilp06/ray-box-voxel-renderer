#include "world.hpp"
#include <iostream>
#include <algorithm>
#include <deque>
#include <unordered_set>
#include <queue>

#include <tracy/Tracy.hpp>

World::World()
{
    noise.SetSeed(1337);
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFrequency(0.01);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(5);
    noise.SetFractalLacunarity(2.0);
    noise.SetFractalGain(0.5);
}

std::shared_ptr<Chunk> World::GenerateChunk(glm::ivec3 chunk_position)
{
    ZoneScoped;

    auto chunk = std::make_shared<Chunk>();

    int base_height = 32;
    int scale = 16;

    for(int x = 0; x < CHUNK_SIZE; x++)
    {
        for(int z = 0; z < CHUNK_SIZE; z++)
        {
            int height = noise.GetNoise(static_cast<float>(chunk_position.x * CHUNK_SIZE + x), static_cast<float>(chunk_position.z * CHUNK_SIZE + z)) * scale + base_height;
            for(int y = 0; y < height; y++)
            {
                int converted_height = y + chunk_position.y * CHUNK_SIZE;
                if(converted_height < 0)
                {
                    continue;
                }
                if(converted_height >= WORLD_HEIGHT_IN_CHUNKS * CHUNK_SIZE)
                {
                    continue;
                }

                if(converted_height < height)
                {
                    chunk->SetBlock(x, y, z, BlockManager::GetBlockIndex("stone"));
                }
                else if(converted_height == height)
                {
                    chunk->SetBlock(x, y, z, BlockManager::GetBlockIndex("grass"));
                }
            }
            
        }
    }

    return chunk;
}

void World::RequestNewChunk(glm::ivec3 chunk_position)
{
    ZoneScoped;

    std::lock_guard<std::mutex> lock(chunk_mutex);

    if (loaded_chunks.find(chunk_position) != loaded_chunks.end())
    {
        return;
    }

    if (generating_chunks.find(chunk_position) != generating_chunks.end())
    {
        return;
    }
    // std::cout << "Chunk requested at " << chunk_position.x << " " << chunk_position.y << " " << chunk_position.z << std::endl;

    generating_chunks[chunk_position] = std::async(std::launch::async, &World::GenerateChunk, this, chunk_position);
}

void World::Update(glm::vec3 player_position)
{
    ZoneScoped;
    this->player_position = player_position;
    glm::ivec3 new_chunk_pos = glm::floor(player_position / static_cast<float>(CHUNK_SIZE));
    if (new_chunk_pos != player_chunk)
    {
        player_chunk = new_chunk_pos;
        UpdateNearbyChunks(player_position);
    }

    // check for completed futures

    std::lock_guard<std::mutex> lock(chunk_mutex);
    for (auto futures = generating_chunks.begin(); futures != generating_chunks.end();)
    {
        auto &future_chunk = futures->second;
        if (future_chunk.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {

            std::shared_ptr<Chunk> chunk = future_chunk.get();
            loaded_chunks.try_emplace(futures->first, chunk);
            futures = generating_chunks.erase(futures);
            // std::cout << "Chunk loaded at " << futures->first.x << " " << futures->first.y << " " << futures->first.z << std::endl;
        }
        else
        {
            futures++;
        }
    }
}

bool World::ChunkLoaded(glm::ivec3 chunk_position)
{
    ZoneScoped;

    std::lock_guard<std::mutex> lock(chunk_mutex);
    return loaded_chunks.find(chunk_position) != loaded_chunks.end();
}

void World::UpdateNearbyChunks(glm::vec3 player_position)
{
    ZoneScoped;

    for (int x = -CHUNK_DISTANCE; x < CHUNK_DISTANCE; x++)
    {
        for (int y = -CHUNK_DISTANCE; y < CHUNK_DISTANCE; y++)
        {
            for (int z = -CHUNK_DISTANCE; z < CHUNK_DISTANCE; z++)
            {
                glm::ivec3 chunk_offset(x, y, z);
                glm::ivec3 chunk_position = player_chunk + chunk_offset;

                if (chunk_position.y < 0 || chunk_position.y >= WORLD_HEIGHT_IN_CHUNKS)
                {
                    continue;
                }
                if (abs(chunk_offset.x) < CHUNK_DISTANCE || abs(chunk_offset.y) < CHUNK_DISTANCE || abs(chunk_offset.z) < CHUNK_DISTANCE)
                {
                    RequestNewChunk(chunk_position);
                }
                else
                {
                    if (ChunkLoaded(chunk_position))
                    {
                        std::lock_guard<std::mutex> lock(chunk_mutex);

                        loaded_chunks.erase(chunk_position);
                    }
                }
            }
        }
    }
}

int World::EvaluateLoD(glm::ivec3 chunk_position)
{
    glm::ivec3 offset = chunk_position - player_chunk;
    
    if(glm::abs(offset.x) < LOD_RING_1 && glm::abs(offset.y) < LOD_RING_1 && glm::abs(offset.z) < LOD_RING_1)
    {
        return 1;
    }
    else if(glm::abs(offset.x) < LOD_RING_2 && glm::abs(offset.y) < LOD_RING_2 && glm::abs(offset.z) < LOD_RING_2)
    {
        return 2;
    }
    else if(glm::abs(offset.x) < LOD_RING_3 && glm::abs(offset.y) < LOD_RING_3 && glm::abs(offset.z) < LOD_RING_3)
    {
        return 3;
    }
    else
    {
        return 4;
    }
}
