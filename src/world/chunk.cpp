#include <world/chunk.hpp>
#include <vector>
#include <iostream>

Chunk::Chunk()
{
    // initialize to zero
    for (uint16_t i = 0; i < CHUNK_VOLUME; i++)
    {
        blocks[i] = 0;
    }
}

void Chunk::SetBlock(uint16_t x, uint16_t y, uint16_t z, uint8_t block_manager_index)
{
    blocks[interleaveBits(x, y, z)] = block_manager_index;
}

void Chunk::SetBlock(uint16_t x, uint16_t y, uint16_t z, const std::string &block_name)
{
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
    {
        throw std::runtime_error("SetBlock::Block out of bounds");
    }
    SetBlock(x, y, z, BlockManager::GetBlockIndex(block_name));
}

uint8_t &Chunk::GetBlock(uint16_t x, uint16_t y, uint16_t z)
{
    // TODO: insert return statement here
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
    {
        throw std::runtime_error("GetBlock::Block out of bounds");
    }
    return blocks[interleaveBits(x, y, z)];
}

uint8_t *Chunk::GetBlocks()
{
    return blocks;
}

// CompressedChunk::CompressedChunk(UncompressedChunk &uncompressed_chunk)
// {
//     uint8_t *blocks = uncompressed_chunk.GetBlocks();
//     uint8_t current_block = blocks[0];
//     uint16_t current_count = 1;

//     // iterate over z-order

//     for (uint16_t i = 1; i < CHUNK_VOLUME; i++)
//     {
//         if (blocks[i] == current_block)
//         {
//             current_count++;
//         }
//         else
//         {
//             data.push_back(current_block);
//             data.push_back(current_count);
//             current_block = blocks[i];
//             current_count = 1;
//         }

//         if (i == CHUNK_VOLUME - 1)
//         {
//             data.push_back(current_block);
//             data.push_back(current_count);
//         }
//     }
// }

// UncompressedChunk CompressedChunk::Decompress()
// {

//     UncompressedChunk chunk;
//     uint16_t i = 0;
//     uint16_t decomp_index = 0;
//     while (i + 1 < data.size())
//     {
//         uint8_t block = data[i];
//         uint16_t count = data[i + 1];
//         for (uint16_t j = 0; j < count; j++)
//         {
//             chunk.GetBlocks()[decomp_index + j] = block;
//         }
//         decomp_index += count;
//         i += 2;
//     }

//     return chunk;
// }