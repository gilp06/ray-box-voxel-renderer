#pragma once

#include <world/block_type.hpp>
#include <vector>

constexpr uint8_t CHUNK_SIZE = 16;
// 16x16x16

class UncompressedChunk
{
public:
    UncompressedChunk();

    void SetBlock(uint16_t x, uint16_t y, uint16_t z, uint8_t block_manager_index);
    void SetBlock(uint16_t x, uint16_t y, uint16_t z, const std::string &block_name);
    uint8_t &GetBlock(uint16_t x, uint16_t y, uint16_t z);
    uint8_t *GetBlocks();

private:
    uint8_t blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
};

class CompressedChunk
{
public:
    CompressedChunk() = default;
    CompressedChunk(UncompressedChunk &uncompressed_chunk);
    UncompressedChunk Decompress();
    // rle compressed data
    std::vector<uint16_t> data;
};