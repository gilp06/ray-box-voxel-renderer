#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <gfx/shader.hpp>
#include <gfx/cam.hpp>
#include <gfx/chunk_renderer.hpp>
#include <world/world.hpp>

class AppState
{
public:
    AppState();
    void Run();
    void Input();
    void Update();
    void Render();

    Camera camera;
    World w;
    
    ChunkRenderer* chunk_renderer = nullptr;

    GLFWwindow *window = nullptr;
    GLuint ubo = 0;
    float last_frame = 0.0f;
    float delta_time = 0.0f;

    int width = 800;
    int height = 600;

    double last_x;
    double last_y;

    bool first_mouse = true;
};