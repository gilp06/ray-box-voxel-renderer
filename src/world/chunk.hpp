#pragma once

#include <world/block_type.hpp>
#include <vector>

constexpr uint8_t CHUNK_SIZE = 32;
constexpr uint32_t CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

class Chunk
{
public:
    Chunk();

    void SetBlock(uint16_t x, uint16_t y, uint16_t z, uint8_t block_manager_index);
    void SetBlock(uint16_t x, uint16_t y, uint16_t z, const std::string &block_name);
    uint8_t &GetBlock(uint16_t x, uint16_t y, uint16_t z);
    uint8_t *GetBlocks();

private:
    uint8_t blocks[CHUNK_VOLUME];
};

// class CompressedChunk
// {
// public:
//     CompressedChunk() = default;
//     CompressedChunk(UncompressedChunk &uncompressed_chunk);
//     UncompressedChunk Decompress();
//     // rle compressed data
//     std::vector<uint16_t> data;
// };