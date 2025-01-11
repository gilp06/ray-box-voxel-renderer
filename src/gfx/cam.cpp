#include "cam.hpp"

#include <iostream>

Camera::Camera(glm::vec3 pos, float fov, float aspect, float near, float far, float yaw, float pitch)
{
    this->pos = pos;
    this->fov = fov;
    this->aspect = aspect;
    this->near = near;
    this->far = far;

    this->yaw = yaw;
    this->pitch = pitch;

    this->front = glm::vec3(0.0f, 0.0f, 0.0f);
    this->up = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAtRH(pos, pos + front, up);
}

glm::mat4 Camera::GetProjectionMatrix()
{
    // std::cout << "fov: " << fov << " aspect: " << aspect << " near: " << near << " far: " << far << std::endl;
    return glm::perspective(glm::radians(fov), aspect, near, far);
}

void Camera::SetPosition(glm::vec3 pos)
{
    this->pos = pos;
}

void Camera::SetRotation(float yaw, float pitch)
{

    this->yaw = yaw;
    this->pitch = pitch;
    // generate front vector
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(front);
    // std::cout << front.x << " " << front.y << " " << front.z << std::endl;
}

void Camera::ProcessInputs(GLFWwindow *window, float delta_time, float x_offset, float y_offset)
{
    const float cameraSpeed = 10.0f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        pos += cameraSpeed * front * delta_time;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        pos -= cameraSpeed * front * delta_time;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        pos -= glm::normalize(glm::cross(front, up)) * cameraSpeed * delta_time;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        pos += glm::normalize(glm::cross(front, up)) * cameraSpeed * delta_time;
    
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        pos += glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed * delta_time;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        pos -= glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed * delta_time;

    float sensitivity = 0.1f;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch += y_offset;

    if (pitch > 89.0f)
        pitch = 89.0f;

    if (pitch < -89.0f)
        pitch = -89.0f;

    SetRotation(yaw, pitch);
}

void Camera::UpdateAspectRatio(float aspect)
{
    this->aspect = aspect;
}