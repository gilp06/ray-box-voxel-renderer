#include <json/json.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#include <glad/gl.h>

#pragma once

Json::Value LoadJsonFile(const std::string &filename);
std::vector<std::string> GetFilesInDirectory(const std::string &directory);
uint32_t PackColorToInt(int color[4]);

uint16_t interleaveBits(uint16_t x, uint16_t y, uint16_t z);
uint16_t deinterleaveBits(uint16_t morton, uint16_t index);

void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param);