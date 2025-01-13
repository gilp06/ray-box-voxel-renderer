#include "world.hpp"
#include <iostream>
#include <algorithm>
#include <deque>

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
    for (uint16_t i = 0; i < CHUNK_VOLUME; i++)
    {
        if (position.y > WORLD_HEIGHT_IN_CHUNKS - 2)
        {
            chunk.GetBlocks()[i] = BlockManager::GetBlockIndex("grass");
        }
        else
        {
            chunk.GetBlocks()[i] = BlockManager::GetBlockIndex("air");
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

    for (int x = -CHUNK_DISTANCE; x <= CHUNK_DISTANCE; x++)
    {
        for (int z = -CHUNK_DISTANCE; z <= CHUNK_DISTANCE; z++)
        {
            for (int y = -CHUNK_DISTANCE; y <= CHUNK_DISTANCE; y++)
            {
                glm::ivec3 chunk_pos = player_chunk + glm::ivec3(x, y, z);
                // std::cout << "Attempted loading chunk at " << chunk_pos.x << " " << chunk_pos.y << " " << chunk_pos.z << std::endl;

                if (chunk_pos.y < 0 || chunk_pos.y >= WORLD_HEIGHT_IN_CHUNKS)
                    continue;
                if (ChunkLoaded(chunk_pos))
                {
                    continue;
                }

                if (std::find(chunks_to_load.begin(), chunks_to_load.end(), chunk_pos) == chunks_to_load.end())
                    chunks_to_load.push_back(chunk_pos);
            }
        }
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
