#include <app.hpp>
#include <utils/utils.hpp>
#include <world/chunk.hpp>
#include <world/world.hpp>
#include <chrono>
#include <variant>

int main()
{
    // World w;

    // auto start = std::chrono::high_resolution_clock::now();
    // w.NewChunk({0, 0, 0});
    // auto end = std::chrono::high_resolution_clock::now();

    // std::cout << "Time to create chunk: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    // // add blocks to chunk

    // UncompressedChunk &chunk = std::get<std::reference_wrapper<UncompressedChunk>>(w.GetChunk({0, 0, 0}));
    // start = std::chrono::high_resolution_clock::now();
    // for (int x = 0; x < CHUNK_SIZE; x++)
    // {
    //     for (int y = 0; y < CHUNK_SIZE; y++)
    //     {
    //         for (int z = 0; z < CHUNK_SIZE; z++)
    //         {
    //             chunk.SetBlock(x, y, z, 1);
    //         }
    //     }
    // }
    // end = std::chrono::high_resolution_clock::now();

    // std::cout << "Time to set blocks: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    // start = std::chrono::high_resolution_clock::now();
    // w.DeactivateChunk({0, 0, 0});
    // end = std::chrono::high_resolution_clock::now();

    // std::cout << "Time to deactivate chunk: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    // start = std::chrono::high_resolution_clock::now();
    // w.ActivateChunk({0, 0, 0});
    // end = std::chrono::high_resolution_clock::now();

    // std::cout << "Time to activate chunk: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    // UncompressedChunk &chunk2 = std::get<std::reference_wrapper<UncompressedChunk>>(w.GetChunk({0, 0, 0}));

    // SVO svo;
    // svo.Insert({0, 15, 15}, 1);
    // svo.Insert({0, 14, 15}, 2);
    // std::cout << (uint32_t) svo.Get({0, 15, 15}) << std::endl;
    // std::cout << (uint32_t) svo.Get({0, 14, 15}) << std::endl;

    // SVO svo;
    // svo.Insert({0, 15, 15}, 1);
    // svo.Insert({0, 14, 15}, 2);

    // std::cout << *svo.Find({0, 15, 15}) << std::endl;
    // std::cout << *svo.Find({0, 14, 15}) << std::endl;


    AppState app;
    app.Run();
}