#include <app.hpp>
#include <utils/utils.hpp>
#include <world/chunk.hpp>
#include <world/world.hpp>
#include <chrono>
#include <variant>
int main()
{
    World w;

    auto start = std::chrono::high_resolution_clock::now();
    w.NewChunk({0, 0, 0});
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Time to create chunk: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    // add blocks to chunk

    UncompressedChunk &chunk = std::get<UncompressedChunk &>(w.GetChunk({0, 0, 0}));
    chunk.SetBlock(0, 0, 0, "dirt");
    chunk.SetBlock(0, 1, 0, "dirt");
    chunk.SetBlock(0, 2, 0, "dirt");

    start = std::chrono::high_resolution_clock::now();
    w.DeactivateChunk({0, 0, 0});
    end = std::chrono::high_resolution_clock::now();

    std::cout << "Time to deactivate chunk: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    w.ActivateChunk({0, 0, 0});
    end = std::chrono::high_resolution_clock::now();

    std::cout << "Time to activate chunk: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    // AppState app;
    // app.Run();
}