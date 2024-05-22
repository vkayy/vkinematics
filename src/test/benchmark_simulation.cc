#include <benchmark/benchmark.h>
#include <SFML/Graphics.hpp>

#include "../simulation/simulation.hpp"

static void BM_updateSimulation(benchmark::State &state) {
    int32_t window_width = 1920;
    int32_t window_height = 1080;
    float max_object_count = 500;
    float radius = 10.0f;
    float max_angle = 1.0f;
    bool speed_colouring = state.range(4);
    float spawn_delay = 0.005f;
    float spawn_speed = 10.0f;
    int32_t framerate_limit = 60;
    int32_t thread_count = state.range(2);
    int32_t substeps = 8;
    bool gravity_on = state.range(3);
    int32_t collision_resolver = state.range(1);
    
    for (auto _ : state) {
        tp::ThreadPool thread_pool(thread_count);
        Solver solver(
            sf::Vector2f(window_width, window_height),
            substeps,
            radius,
            framerate_limit,
            speed_colouring,
            thread_pool,
            gravity_on
        );
        sf::Clock clock;
        RNG<float> rng;
        for (int i = 0; i < max_object_count; ++i) {
            sf::Vector2f spawn_position(rng.getRange(window_width), rng.getRange(window_height));
            VerletObject& object = solver.addObject(spawn_position, radius);
            const float t = solver.time;
            object.colour = getRainbowColour(t);
            const float angle = max_angle * sin(t) + M_PI * 0.5f;
            solver.setObjectVelocity(object, spawn_speed * sf::Vector2f{cos(angle), sin(angle)});
        }
        for (int i = 0; i < state.range(0); ++i) {
            switch (collision_resolver) {
                case 2: solver.updateNaive(); break;
                case 1: solver.updateCellular(); break;
                default: solver.updateThreaded();
            }
        }
    }
    state.SetComplexityN(state.range(0));
}

/*

Benchmark format is as follows:

BENCHMARK(BM_updateSimulation)
->Name([test name])
->ArgsProduct({
    [number of updates to test],
    [collision resolver selection],
    [number of threads to use],
    [gravity on/off],
    [colouring method],
});

*/

BENCHMARK(BM_updateSimulation)
->Name("thread_count")
->ArgsProduct({
    {500},
    {0},
    benchmark::CreateDenseRange(2, 16, 1),
    {0},
    {0},
})
->MeasureProcessCPUTime();

BENCHMARK(BM_updateSimulation)
->Name("resolver_brute_force")
->ArgsProduct({
    {5, 10, 25, 50, 100, 250, 500, 1000},
    {2},
    {1},
    {0, 1},
    {0},
})
->Complexity()
->MeasureProcessCPUTime();

BENCHMARK(BM_updateSimulation)
->Name("reslolver_spatial_partitioning")
->ArgsProduct({
    {5, 10, 25, 50, 100, 250, 500, 1000},
    {1},
    {1},
    {0, 1},
    {0},
})
->Complexity()
->MeasureProcessCPUTime();

BENCHMARK(BM_updateSimulation)
->Name("resolver_multithreaded")
->ArgsProduct({
    {5, 10, 25, 50, 100, 250, 500, 1000},
    {0},
    {5, 6},
    {0, 1},
    {0},
})
->Complexity()
->MeasureProcessCPUTime();

BENCHMARK(BM_updateSimulation)
->Name("colouring_method")
->ArgsProduct({
    {500},
    {0},
    {5},
    {0},
    {0, 1},
});

BENCHMARK_MAIN();