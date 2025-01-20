#include <app.hpp>

#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include "world/block_type.hpp"
#include <chrono>
#include <random>
#include <utils/utils.hpp>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

// callback handlers

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    auto *app = reinterpret_cast<AppState *>(glfwGetWindowUserPointer(window));

    if (app == nullptr)
    {
        return;
    }

    app->width = width;
    app->height = height;
}

AppState::AppState() : camera(glm::vec3(0.0f, 0.0f, 0.0f), 90.0f, 640.0f / 480.0f, 0.1f, 10000.0f, 90.0f, 0.0f)
{
    // World Initialization
    BlockManager::RegisterBlock("air", {BlockType::Empty, 0});
    BlockManager::RegisterDirectory("resources/blocks");

    std::random_device rd;
    int count = 0;
    // for (int x = -10; x < 10; x++)
    // {
    //     for (int z = -10; z < 10; z++)
    //     {
    //         Chunk& chunk = w.NewChunk(glm::ivec3(x, 0, z));
    //         // fill with random blocks
    //         for (uint16_t i = 0; i < CHUNK_VOLUME; i++)
    //         {
    //             chunk.GetBlocks()[i] = BlockManager::GetBlockIndex("grass");
    //         }
    //         count++;
    //         std::cout<< "Chunk " << count << " created" << std::endl;
    //     }
    // }

    // Render Initialization

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Window", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwGetCursorPos(window, &last_x, &last_y);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    gladLoadGL(glfwGetProcAddress);
    glViewport(0, 0, width, height);

    // features
    glEnable(GL_PROGRAM_POINT_SIZE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
#endif
    TracyGpuContext;
    // // uniform buffer initialization

    GLuint ubo;
    glGenBuffers(1, &ubo);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(glm::mat4) + sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    // chunk_renderer = new ChunkRenderer(w);
    world = std::make_shared<World>();
    chunk_renderer = std::make_shared<ChunkRenderer>(world);

}

void AppState::Input()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // f11 for fullscreen

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    float xoffset = xpos - last_x;
    float yoffset = last_y - ypos; // reversed since y-coordinates go from bottom to top

    if (first_mouse)
    {
        xoffset = 0;
        yoffset = 0;
        first_mouse = false;
    }

    last_x = xpos;
    last_y = ypos;

    camera.ProcessInputs(window, delta_time, xoffset, yoffset);
}

void AppState::Update()
{
    ZoneScoped;
    camera.UpdateAspectRatio(width / (float)height);
    glm::mat4 proj = camera.GetProjectionMatrix();
    glm::mat4 view = camera.GetViewMatrix();

    glm::mat4 view_rot = glm::mat4(glm::mat3(view));
    glm::mat4 view_pos = glm::translate(glm::mat4(1.0f), -camera.pos);

    // update world load

    world->Update(camera.pos);
    chunk_renderer->Update();
    // chunk_renderer->AddChunk(glm::ivec3(0, 0, 0));

    // std::cout << camera.pos.x << " " << camera.pos.y << " " << camera.pos.z << std::endl;
    // print full view matrix
    // for (int i = 0; i < 4; i++)
    // {
    //     for (int j = 0; j < 4; j++)
    //     {
    //         std::cout << view[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }

    // framerate
    // if one second has passed print framerate
    static auto last_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_time);
    if (duration.count() >= 1)
    {
        std::cout << "FPS: " << 1 / delta_time << std::endl;
        last_time = current_time;
    }

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view_rot));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(view_pos));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, sizeof(glm::mat4), glm::value_ptr(glm::inverse(proj * view_rot)));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 4, sizeof(glm::vec4), glm::value_ptr(glm::vec4(0, 0, width, height)));
}

void AppState::Render()
{
    ZoneScoped;
    TracyGpuZone("Render");
    // glBindVertexArray(vao);
    // shader->Use();
    // glDrawArrays(GL_POINTS, 0, 3);
    chunk_renderer->Render(this->camera);
}

void AppState::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        FrameMarkStart("Frame");
        glClearColor(0.7, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwPollEvents();

        Input();
        Update();
        Render();

        glfwSwapBuffers(window);
        TracyGpuCollect;

        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
        FrameMarkEnd("Frame");
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}