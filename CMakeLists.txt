cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)

project(voxel_terrain)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad
    GIT_TAG        v2.0.6
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    message("Fetching glad")
    FetchContent_MakeAvailable(glad)

    add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
    glad_add_library(glad REPRODUCIBLE EXCLUDE_FROM_ALL LOADER API gl:core=4.6 EXTENSIONS GL_NV_mesh_shader GL_KHR_shader_subgroup)
endif()

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glm)

set(JSONCPP_WITH_TESTS OFF CACHE BOOL "Enable jsoncpp tests" FORCE)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG 1.9.6
)
FetchContent_MakeAvailable(json)

add_custom_target(shaders
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/resources
        COMMENT "copying ${CMAKE_SOURCE_DIR}/resources to ${CMAKE_BINARY_DIR}/VERSION/resources"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

# i know you're not supposed to do this but i'm lazy
set(SOURCES 
    src/main.cpp
    src/app.cpp
    src/utils/utils.cpp
    src/gfx/cam.cpp
    src/gfx/shader.cpp
    src/gfx/chunk_renderer.cpp
    src/world/chunk.cpp
    src/world/block_type.cpp
    src/world/world.cpp
)

add_executable(voxel_terrain ${SOURCES})
add_dependencies(voxel_terrain shaders)
target_link_libraries(voxel_terrain PRIVATE glad glfw glm jsoncpp_object)
target_include_directories(voxel_terrain PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

