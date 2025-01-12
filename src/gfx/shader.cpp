#include "shader.hpp"

#include <iostream>
#include <fstream>

ShaderProgram::ShaderProgram()
{
    program = glCreateProgram();
    shaders = std::vector<GLuint>();
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(program);

    for (auto shader : shaders)
    {
        glDeleteShader(shader);
    }
}

void ShaderProgram::AddShader(const std::string &shader_file, GLenum type)
{
    GLuint shader = glCreateShader(type);
    shaders.push_back(shader);

    std::ifstream file(shader_file);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << shader_file << std::endl;
        exit(-1);
    }

    std::string source;
    std::string line;
    while (std::getline(file, line))
    {
        source += line + "\n";
    }

    const char *c_source = source.c_str();
    glShaderSource(shader, 1, &c_source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        std::cerr << "Failed to compile shader: " << info_log << std::endl;
        exit(-1);
    }

    glAttachShader(program, shader);
}

void ShaderProgram::Build()
{
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        std::cerr << "Failed to link program: " << info_log << std::endl;
        exit(-1);
    }

    for (auto shader : shaders)
    {
        glDeleteShader(shader);
    }
}

ShaderProgram::ShaderProgram(const std::string &vertex_shader, const std::string &fragment_shader)
{
    program = glCreateProgram();
    shaders = std::vector<GLuint>();

    AddShader(vertex_shader, GL_VERTEX_SHADER);
    AddShader(fragment_shader, GL_FRAGMENT_SHADER);
    Build();
}

void ShaderProgram::Use()
{
    glUseProgram(program);
}

void ShaderProgram::SetUniformBlock(const std::string &block_name, GLuint binding_point)
{
    GLuint block_index = glGetUniformBlockIndex(program, block_name.c_str());
    glUniformBlockBinding(program, block_index, binding_point);
}

void ShaderProgram::SetUniformIVec3(const std::string &name, const glm::ivec3 &value)
{
    GLint location = glGetUniformLocation(program, name.c_str());
    glUniform3iv(location, 1, glm::value_ptr(value));
}
