#include "utils.hpp"

std::vector<std::string> GetFilesInDirectory(const std::string &directory)
{
    std::filesystem::path p(directory);
    if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
    {
        return std::vector<std::string>();
    }
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(p))
    {
        const auto full_name = entry.path().string();
        if (entry.is_regular_file())
        {
            files.push_back(full_name);
        }
    }
    return files;
}
uint32_t PackColorToInt(int color[4])
{
    return (color[0] << 24) | (color[1] << 16) | (color[2] << 8) | color[3];
};

Json::Value LoadJsonFile(const std::string &filename)
{
    std::ifstream file(filename, std::ifstream::binary);
    Json::Value root;
    file >> root;
    return root;
};

uint16_t interleaveBits(uint16_t x, uint16_t y, uint16_t z)
{
    x &= 0x1F;
    y &= 0x1F;
    z &= 0x1F;

    auto splitBy3 = [](uint16_t a) -> uint16_t
    {
        uint16_t x = a & 0x1F; // 5 bits
        x = (x | x << 8) & 0xF00F;
        x = (x | x << 4) & 0x30C30C3;
        x = (x | x << 2) & 0x9249249;
        return x;
    };

    uint16_t Morton_x = splitBy3(x) << 2;
    uint16_t Morton_y = splitBy3(y) << 1;
    uint16_t Morton_z = splitBy3(z);

    // Combine the separated bits to form the Morton code
    return Morton_x | Morton_y | Morton_z;
}

// i asked chatgpt help what the hell is going on here
uint16_t deinterleaveBits(uint16_t morton, uint16_t index)
{
    morton >>= index;
    morton &= 0x9249249;
    morton |= morton >> 2;
    morton &= 0x30C30C3;
    morton |= morton >> 4;
    morton &= 0xF00F;
    morton |= morton >> 8;
    morton &= 0x1F;
    return morton;
}