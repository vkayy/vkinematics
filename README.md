# Multithreaded Physics Engine

A 2D physics engine which deterministically resolves and visualises the collisions of thousands of rigid bodies using a custom-written Verlet Integration library.

## How does it work?

This engine uses Verlet Integration to accurately model the movement of rigid bodies in a dynamic system.

In order to handle multithreading, a thread pool is used, parallelising the work of both the broad phase and narrow phases of collision resolution across workers.

Currently, the broad phase involves spatial partitioning into a uniform collision grid, whilst the narrow phase is AABB-based. The linear structure of a uniform grid enables O(1) lookup and an elegant means of multithreading in contrast to the non-linear quadtree or circle tree structures, hence the design choice.

It is important to note that the resolution will only be deterministic if the minimum and maximum radii of objects used in the simulation are the same. This is because a random number generator is used to seed the radii of objects spawned when a range is given.

## How do I use this?

First of all, for the sake of visualisation, SFML has to be locally installed.

On Linux, use your preferred package manager:
```
sudo apt-get install libsfml-dev
```

On MacOS, use Homebrew:
```
brew install sfml
```

On Windows, download SFML from the [SFML website](https://www.sfml-dev.org/download.php).

To run the engine, assuming you've downloaded the repository, `cd` into the directory of the repository, then call `mkdir build`. At this stage, `cd` into the `build` directory, and call these commands:

```
cmake ..
cmake --build .
./multiThreadedPhysicsEngine
```

Now, whenever you want to re-run a simulation, it is as simple as `Up Arrow` then `Enter` (i.e., running that block of commands again).

You can use the keys `A` and `R` to respectively activate an attractor/repeller in the centre of the simulation space.

In `src/main.cpp`, there are also numerous parameters that you can modify to your liking at the top of the file:
- `RENDER_DISPLAY`: If true, the simulation is displayed. Otherwise, it is not.
- `WINDOW_WIDTH`: The width of the window.
- `WINDOW_HEIGHT`: the width of the window.
- `MAX_OBJECT_COUNT`: The maximum object count.
- `MIN_RADIUS`: The minimum object radius.
- `MAX_RADIUS`: The maximum object radius.
- `MAX_ANGLE`: The maximum angle of object spawn velocity.
- `SPEED_COLOURING`: If true, objects are coloured based on speed. By default, they are coloured by rainbow.
- `SPAWN_DELAY`: The delay between consecutive object spawns.
- `SPAWN_SPEED`: The speed at which an object spawns.
- `FRAMERATE_LIMIT`: The maximum framerate.
- `THREAD_COUNT`: The number of threads used (experiment with this, see what works best for you).
- `COLLISION_RESOLVER`: Three choices are available:
    - `0`: Multithreaded and optimised with uniform collision grid spatial partitioning.
    - `1`: Single-threaded and optimised with uniform collision grid spatial partitioning.
    - `2`: Single-threaded and brute force collision resolution.
    - Any other (invalid) option will default to multithreading.
- `GRAVITY_ON`: If true, objects are affected by gravity. Otherwise, they are not.
- `SPAWN_POSITION`: The spawn position of each object. Ensure this is in terms of `WINDOW_WIDTH` and `WINDOW_HEIGHT` to prevent out-of-bounds spawning.

## How is performance measured?

By using Google Benchmark, I wrote a series of simple benchmarks to analyse the performance of various thread counts, resolvers, and other parameters. If you'd like to run the benchmarking locally, `cd` into `src`, then run the following commands:

```
g++ test/benchmark_simulation.cc -std=c++17 -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -lsfml-graphics -lsfml-window -lsfml-system -o test/benchmark_simulation
```
The above compiles the benchmark file (in case you would like to modify or add benchmarks) into an executable file that can be ran for the actual benchmark analysis.


```
test/benchmark_simulation --benchmark_out=test/benchmark_simulation.csv --benchmark_out_format=csv --benchmark_repetitions=5
```
Then, the above runs the benchmark executable, which then causes it to output the results into your console and a `.csv` file. To read more about the various options, check out the [Google Benchmark user guide](https://github.com/google/benchmark/blob/main/docs/user_guide.md).

On one instance of the benchmarking locally, I obtained these Big-Oh complexities (where N is the number of objects):

```
                   BENCHMARK                                       TIME             
resolver_brute_force_objects/process_time_BigO                 59239.43 N^2
resolver_spatial_partitioning_objects/process_time_BigO      4058967.33 N
resolver_multithreaded_objects/process_time_BigO             1581379.74 N
```

As the brute-force algorithm is of quadratic time complexity with respect to the number of objects, mean operation time unsurprisingly scales quadratically as the number of objects increases. On the other hand, by using spatial partitioning with some pruning, we achieve a linear time complexity, which is then improved further by a constant through multithreading.

Therefore, the brute-force algorithm is significantly less efficient than both the spatial-partitioning algorithm, with or without multithreading. On the other hand, the multithreaded algorithm appears to be just over 60% faster than the spatial-partitioning algorithm.

## What are the next steps?

At the moment, the engine simulates 2D physics only, but, with access to greater processing power, I may extend the engine to simulate 3D physics.
