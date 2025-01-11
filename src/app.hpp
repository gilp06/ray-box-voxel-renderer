#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <shader.hpp>
#include <cam.hpp>

class AppState
{
public:
    AppState();
    void Run();
    void Input();
    void Init();
    void Update();
    void Render();

    GLFWwindow *window = nullptr;
    ShaderProgram* shader = nullptr;
    
    GLuint vao = 0;
    GLuint chunk_vbo = 0;
    GLuint ubo = 0;

    Camera camera;

    float last_frame = 0.0f;
    float delta_time = 0.0f;

    int width = 800;
    int height = 600;

    double last_x;
    double last_y;

    bool first_mouse = true;
};