#include "utils.hpp"
#include <iostream>
#include <queue>
#include <set>

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

void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
	auto const src_str = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
        default: return "UNKNOWN";
		}
	}();

	auto const type_str = [type]() {
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
        default: return "UNKNOWN";
		}
	}();

	auto const severity_str = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
        default: return "UNKNOWN";
		}
	}();
	std::cout << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
}

void test_loop(int max_distance)
{

    // get all positions within the cube centered at (0,0,0) and expanding in each direction by max_distance
    // preferably in a spiral pattern with the center being (0,0,0) and no duplicates
    // print the positions in the order they are visited


    int total = 0;
    std::set<std::tuple<int, int, int>> visited;
    std::queue<std::tuple<int, int, int>> to_visit;
    to_visit.push({0, 0, 0});

    while (!to_visit.empty())
    {
        auto [x, y, z] = to_visit.front();
        to_visit.pop();

        if (visited.count({x, y, z}) > 0)
        {
            continue;
        }

        visited.insert({x, y, z});
        total++;
        std::cout << "(" << x << ", " << y << ", " << z << ")\n";

        if (std::abs(x) < max_distance)
        {
            to_visit.push({x + 1, y, z});
            to_visit.push({x - 1, y, z});
        }
        if (std::abs(y) < max_distance)
        {
            to_visit.push({x, y + 1, z});
            to_visit.push({x, y - 1, z});
        }
        if (std::abs(z) < max_distance)
        {
            to_visit.push({x, y, z + 1});
            to_visit.push({x, y, z - 1});
        }
    }

    std::cout << "Total: " << total << std::endl;
}
