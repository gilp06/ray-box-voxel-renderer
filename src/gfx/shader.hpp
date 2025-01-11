#include <glad/gl.h>
#include <GLFW/glfw3.h>

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
    private:
        GLuint program = 0;
        std::vector<GLuint> shaders;
};