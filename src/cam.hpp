#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>


// perspective camera
class Camera
{
public:
    Camera(glm::vec3 pos, float fov, float aspect, float near, float far, float yaw, float pitch);
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();

    void SetPosition(glm::vec3 pos);
    void SetRotation(float yaw, float pitch);
    void ProcessInputs(GLFWwindow *window, float delta_time, float x_offset, float y_offset);
    void UpdateAspectRatio(float aspect);

    float yaw;
    float pitch;
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;

private:
    float fov;
    float aspect;
    float near;
    float far;
};