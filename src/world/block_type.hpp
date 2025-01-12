#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <utils/utils.hpp>

enum BlockType
{
    Solid,
    Empty,
};

struct BlockData
{
    BlockType type;
    uint32_t color;
};

// manages all block types, and provides a way to get their block data.
class BlockManager
{
public:
    static void RegisterBlock(const std::string& name, BlockData block_data);
    static void RegisterDirectory(const std::string &directory); // gets all block jsons from a directory

    static BlockData GetBlockData(const std::string& name);
    static uint8_t GetBlockIndex(const std::string& name);
    static BlockData GetBlockData(uint8_t index);

public:
    static std::vector<BlockData> block_datas;
    static std::unordered_map<std::string, uint8_t> block_index_map;
};