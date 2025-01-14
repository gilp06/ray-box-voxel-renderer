#include "world.hpp"
#include <iostream>
#include <algorithm>
#include <deque>
#include <unordered_set>
#include <queue>

World::World()
{
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

Chunk &World::GetChunk(glm::ivec3 position)
{

    auto active_chunk = active_chunks.find(position);
    if (active_chunk != active_chunks.end())
    {
        return active_chunk->second;
    }

    // auto inactive_chunk = inactive_chunks.find(position);
    // if (inactive_chunk != inactive_chunks.end())
    // {
    //     return inactive_chunk->second;
    // }

    throw std::runtime_error("Chunk not found");
}

Chunk &World::NewChunk(glm::ivec3 position)
{
    // std::cout << "Loading new chunk!" << std::endl;
    // create new chunk
    Chunk chunk;
    // set all blocks to air
    
    // base height 
    int base_height = 32;
    // noise height
    int noise_height = 8; // max modulation

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for(int z = 0; z < CHUNK_SIZE; z++)
        {
            // get heightmap value
            float height = noise.GetNoise((position.x * CHUNK_SIZE + x) / 1.0f, (position.z * CHUNK_SIZE + z) / 1.0f);
            float height2 = noise.GetNoise((position.x * CHUNK_SIZE + x) / 2.0f, (position.z * CHUNK_SIZE + z) / 2.0f);
            float height3 = noise.GetNoise((position.x * CHUNK_SIZE + x) / 4.0f, (position.z * CHUNK_SIZE + z) / 4.0f);

            height = height + height2 * 0.5f + height3 * 0.25f;
            height = height * (noise_height - 0.5) + base_height;
            int h = (int)height;
            // fill with stone

            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                int world_y = position.y * CHUNK_SIZE + y;
                if(world_y < h)
                {
                    chunk.SetBlock(x, y, z, BlockManager::GetBlockIndex("stone"));
                }
                else if(world_y == h || world_y == h+1)
                {
                    chunk.SetBlock(x, y, z, BlockManager::GetBlockIndex("grass"));
                }
            }
        }
    }

    

    // insert into active chunks
    // std::cout << "Loaded chunk at " << position.x << " " << position.y << " " << position.z << std::endl;
    active_chunks.try_emplace(position, chunk);
    return active_chunks.at(position);
}

void World::UnloadChunk(glm::ivec3 position)
{
    // remove chunk from inactive and active chunks
    // check if chunk is active
    if (active_chunks.find(position) == active_chunks.end())
    {
        return;
    }
    active_chunks.erase(position);
    // std::cout << "Unloaded chunk at " << position.x << " " << position.y << " " << position.z << std::endl;
    // inactive_chunks.erase(position);
}

bool World::ChunkLoaded(glm::ivec3 position)
{
    return active_chunks.find(position) != active_chunks.end();
}

// void World::ActivateChunk(glm::ivec3 position)
// {
//     // get chunk
//     auto chunk = inactive_chunks.find(position);
//     if (chunk == inactive_chunks.end())
//     {
//         return;
//     }

//     // decompress chunk
//     active_chunks.try_emplace(position, chunk->second.Decompress());
//     // remove from inactive chunks
//     inactive_chunks.erase(position);
// }

// void World::DeactivateChunk(glm::ivec3 position)
// {
//     // get chunk
//     auto chunk = active_chunks.find(position);
//     if (chunk == active_chunks.end())
//     {
//         return;
//     }

//     // compress chunk
//     inactive_chunks.try_emplace(position, CompressedChunk(chunk->second));
//     // remove from active chunks
//     active_chunks.erase(position);
// }

void World::Update(glm::vec3 player_position)
{
    // todo implement async chunk loading and unloading
    // load chunks around player.

    // print currently loaded chunks

    // get player chunk
    glm::ivec3 player_chunk = glm::floor(glm::vec3(player_position) / (float)CHUNK_SIZE);
    // check if player chunk is loaded
    // std::cout << "Player chunk: " << player_chunk.x << " " << player_chunk.y << " " << player_chunk.z << std::endl;
    // check if any chunks are too far from player
    for (auto &chunk : active_chunks)
    {
        // square distance from player chunk
        glm::ivec3 chunk_pos = chunk.first;
        if (abs(chunk_pos.x - player_chunk.x) > CHUNK_DISTANCE ||
            abs(chunk_pos.y - player_chunk.y) > CHUNK_DISTANCE ||
            abs(chunk_pos.z - player_chunk.z) > CHUNK_DISTANCE)
        {
            assert(ChunkLoaded(chunk_pos));
            if (std::find(chunks_to_unload.begin(), chunks_to_unload.end(), chunk_pos) == chunks_to_unload.end())
            {
                chunks_to_unload.push_back(chunk_pos);
            }
        }
    }

    // load chunks within distance

    int max_distance = CHUNK_DISTANCE;
    std::unordered_set<glm::ivec3> visited;
    std::queue<glm::ivec3> to_visit;

    to_visit.push(player_chunk);

    while (!to_visit.empty())
    {
        auto vec = to_visit.front();
        to_visit.pop();

        if (visited.count(vec) > 0)
        {
            continue;
        }

        visited.insert(vec);
        // std::cout << "Attempted loading chunk at " << chunk_pos.x << " " << chunk_pos.y << " " << chunk_pos.z << std::endl;

        if (vec.y < 0 || vec.y >= WORLD_HEIGHT_IN_CHUNKS)
            continue;

        if (std::abs(vec.x) < max_distance)
        {
            to_visit.push(vec + glm::ivec3(1, 0, 0));
            to_visit.push(vec + glm::ivec3(-1, 0, 0));
        }
        if (std::abs(vec.y) < max_distance)
        {
            to_visit.push(vec + glm::ivec3(0, 1, 0));
            to_visit.push(vec + glm::ivec3(0, -1, 0));
        }
        if (std::abs(vec.z) < max_distance)
        {
            to_visit.push(vec + glm::ivec3(0, 0, 1));
            to_visit.push(vec + glm::ivec3(0, 0, -1));
        }

        if (ChunkLoaded(vec))
        {
            continue;
        }

        if (std::find(chunks_to_load.begin(), chunks_to_load.end(), vec) == chunks_to_load.end())
            chunks_to_load.push_back(vec);
    }

    // unload one chunk per frame
    if (!chunks_to_unload.empty())
    {
        glm::ivec3 chunk_pos = chunks_to_unload.front();
        UnloadChunk(chunk_pos);
        NotifyChunkUnload(chunk_pos);
        chunks_to_unload.pop_front();
    }

    // load one chunk per frame
    if (!chunks_to_load.empty())
    {
        glm::ivec3 chunk_pos = chunks_to_load.front();
        NewChunk(chunk_pos);
        NotifyChunkLoad(chunk_pos);
        chunks_to_load.pop_front();
    }
}

void World::UpdateAdjacentChunks(glm::ivec3 position)
{
    constexpr glm::ivec3 directions[] = {
        glm::ivec3(1, 0, 0),
        glm::ivec3(-1, 0, 0),
        glm::ivec3(0, 1, 0),
        glm::ivec3(0, -1, 0),
        glm::ivec3(0, 0, 1),
        glm::ivec3(0, 0, -1),
    };

    for (auto &dir : directions)
    {
        glm::ivec3 new_pos = position + dir;
        if (ChunkLoaded(new_pos))
        {
            NotifyChunkUpdate(new_pos);
        }
    }
}

void World::SubscribeToChunkLoad(ChunkCallback callback)
{
    chunk_load_callbacks.push_back(callback);
}

void World::SubscribeToChunkUnload(ChunkCallback callback)
{
    chunk_unload_callbacks.push_back(callback);
}

void World::SubscribeToChunkUpdate(ChunkCallback callback)
{
    chunk_update_callbacks.push_back(callback);
}

void World::Input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        active_chunks.rehash(200);
    }
}

void World::NotifyChunkLoad(const glm::ivec3 &position)
{
    for (auto &callback : chunk_load_callbacks)
    {
        callback(position);
    }
}

void World::NotifyChunkUnload(const glm::ivec3 &position)
{
    for (auto &callback : chunk_unload_callbacks)
    {
        callback(position);
    }
}

void World::NotifyChunkUpdate(const glm::ivec3 &position)
{
    for (auto &callback : chunk_update_callbacks)
    {
        callback(position);
    }
}
