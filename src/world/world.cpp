#include "world.hpp"

ChunkVariantRef World::GetChunk(glm::ivec3 position)
{
    auto active_chunk = active_chunks.find(position);
    if (active_chunk != active_chunks.end())
    {
        return active_chunk->second;
    }

    auto inactive_chunk = inactive_chunks.find(position);
    if (inactive_chunk != inactive_chunks.end())
    {
        return inactive_chunk->second;
    }

    throw std::runtime_error("Chunk not found");
}

void World::NewChunk(glm::ivec3 position)
{
    // create new chunk
    UncompressedChunk chunk;
    // set all blocks to air
    for (uint16_t i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++)
    {
        chunk.GetBlocks()[i] = BlockManager::GetBlockIndex("air");
    }

    // insert into active chunks
    active_chunks.try_emplace(position, chunk);
}

void World::UnloadChunk(glm::ivec3 position)
{
    // remove chunk from inactive and active chunks
    active_chunks.erase(position);
    inactive_chunks.erase(position);
}

void World::ActivateChunk(glm::ivec3 position)
{
    // get chunk
    auto chunk = inactive_chunks.find(position);
    if (chunk == inactive_chunks.end())
    {
        return;
    }

    // decompress chunk
    active_chunks.try_emplace(position, chunk->second.Decompress());
    // remove from inactive chunks
    inactive_chunks.erase(position);
}

void World::DeactivateChunk(glm::ivec3 position)
{
    // get chunk
    auto chunk = active_chunks.find(position);
    if (chunk == active_chunks.end())
    {
        return;
    }

    // compress chunk
    inactive_chunks.try_emplace(position, CompressedChunk(chunk->second));
    // remove from active chunks
    active_chunks.erase(position);
}

void World::Update(glm::vec3 player_position)
{
    // todo implement async chunk loading and unloading
}
