#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

class ShaderProgram {
    public: 
        ShaderProgram();
        ~ShaderProgram();
        ShaderProgram(const std::string &vertex_shader, const std::string &fragment_shader);
        void Build();
        void AddShader(const std::string &shader_file, GLenum type);
        void Use();
        void SetUniformBlock(const std::string &block_name, GLuint binding_point);

        void SetUniformIVec3(const std::string &name, const glm::ivec3 &value);
        void SetUniformFloat(const std::string &name, const float value);
        void SetUniformUInt(const std::string &name, const uint32_t value);
    private:
        GLuint program = 0;
        std::vector<GLuint> shaders;
};