#include <world/block_type.hpp>

#include <iostream>

std::vector<BlockData> BlockManager::block_datas;
std::unordered_map<std::string, uint8_t> BlockManager::block_index_map;

void BlockManager::RegisterBlock(const std::string &name, BlockData block_data)
{
    std::cout << "Registered Block: " << name << std::endl;
    switch (block_data.type)
    {
    case BlockType::Empty:
        std::cout << "Type: Empty" << std::endl;
        break;
    case BlockType::Solid:
        std::cout << "Type: Solid" << std::endl;
        break;
    }

    block_datas.push_back(block_data);
    block_index_map[name] = block_datas.size() - 1;
}

void BlockManager::RegisterDirectory(const std::string &directory)
{
    // get all files in directory
    std::vector<std::string> files = GetFilesInDirectory(directory);
    // print all files in directory
    for (const auto &file : files)
    {
        std::cout << "Loading block file: " << file << std::endl;
        // load json file
        Json::Value root = LoadJsonFile(file);
        // get block name
        std::string name = root["name"].asString();
        // get block type
        BlockType type = root["block_type"].asString() == "solid" ? Solid : Empty;
        // get block color
        int color[4] = {root["color"][0].asInt(), root["color"][1].asInt(), root["color"][2].asInt(), root["color"][3].asInt()};
        // pack color into int
        uint32_t packed_color = PackColorToInt(color);
        // create block data

        BlockData block = {type, packed_color};
        // register block

        RegisterBlock(name, block);
    }
}

BlockData BlockManager::GetBlockData(const std::string &name)
{
    return block_datas[GetBlockIndex(name)];
}

uint8_t BlockManager::GetBlockIndex(const std::string &name)
{
    // if block does not exist, return air block
    if (block_index_map.find(name) == block_index_map.end())
    {
        return block_index_map["air"];
    }
    return block_index_map[name];
}

BlockData BlockManager::GetBlockData(uint8_t index)
{
    return block_datas[index];
}