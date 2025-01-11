#include <app.hpp>

#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include "world/block_type.hpp"

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

void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar *message,
                     const void *userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

AppState::AppState() : camera(glm::vec3(0.0f, 0.0f, -15.0f), 90.0f, 640.0f / 480.0f, 0.1f, 1000.0f, 90.0f, 0.0f)
{
    // World Initialization
    BlockManager::RegisterBlock("air", {BlockType::Empty, 0});
    BlockManager::RegisterDirectory("resources/blocks");


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
    gladLoadGL(glfwGetProcAddress);
    glViewport(0, 0, width, height);

    // features
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glDebugMessageCallback(MessageCallback, 0);

    // vao initialization
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vbo initialization
    glGenBuffers(1, &chunk_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_vbo);

    // test triangle
    float points[] = {
        // positions         // colors
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Shader initialization
    shader = new ShaderProgram("resources/shaders/chunk.vert.glsl", "resources/shaders/chunk.frag.glsl");

    // // uniform buffer initialization
    GLuint ubo;
    glGenBuffers(1, &ubo);

    shader->SetUniformBlock("ubo", 0);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(glm::mat4) + sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
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
    camera.UpdateAspectRatio(width / (float)height);
    glm::mat4 proj = camera.GetProjectionMatrix();
    glm::mat4 view = camera.GetViewMatrix();

    glm::mat4 view_rot = glm::mat4(glm::mat3(view));
    glm::mat4 view_pos = glm::translate(glm::mat4(1.0f), -camera.pos);

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

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view_rot));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(view_pos));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, sizeof(glm::mat4), glm::value_ptr(glm::inverse(proj * view_rot)));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 4, sizeof(glm::vec4), glm::value_ptr(glm::vec4(0, 0, width, height)));
}

void AppState::Render()
{
    // glBindVertexArray(vao);
    shader->Use();
    glDrawArrays(GL_POINTS, 0, 3);
}

void AppState::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwPollEvents();

        Input();
        Update();
        Render();

        glfwSwapBuffers(window);
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}