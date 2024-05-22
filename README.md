# Multithreaded Physics Engine

A 2D physics engine which deterministically resolves and visualises the collisions of thousands of rigid bodies using a custom-written Verlet Integration library.

## How does it work?

This engine uses Verlet Integration to accurately model the movement of rigid bodies in a dynamic system.

In order to handle multithreading, a thread pool is used, parallelising the work of both the broad phase and narrow phases of collision resolution across workers.

Currently, the broad phase involves spatial partitioning into a uniform collision grid, whilst the narrow phase is AABB-based. The linear structure of a uniform grid enables an elegant means of multithreading, in contrast to the non-linear quadtree or circle tree structures, hence the choice.

## How do I use this?

First of all, for sake of rendering, SFML has to be installed as it is a dependency.

For Linux, use your preferred package manager:
```
sudo apt-get install libsfml-dev
```

For MacOS, use Homebrew:
```
brew install sfml
```

For Windows, download from the website:

https://www.sfml-dev.org/download.php

Next, assuming you've downloaded the repository, to run the engine, `cd` into the directory of the repository, then call `mkdir build`. At this stage, `cd` into the `build` directory, and call these commands:

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
Then, the above runs the benchmark executable, which then causes it to output the results into your console and a `.csv` file. To read about more the various options, check out the Google Benchmark user guide: https://github.com/google/benchmark/blob/main/docs/user_guide.md.

On one instance of the benchmarking locally, I obtained these Big-Oh results across a mean of five repetitions (where N is the number of updates):

```
        BENCHMARK                              TIME             
brute_force/process_time_BigO              30264097.34 N
spatial_partitioning/process_time_BigO     10316746.77 N
multithreaded/process_time_BigO             6653472.30 N
```

Using these results, we can thus approximate that the multithreaded resolution algorithm is about 80% faster than the brute-force resolution algorithm (the raw values themselves are arbitrary, only the ratios matter).

As the brute-force algorithm is of quadratic time complexity with respect to the number of objects, execution time unsurprisingly scales worse as the number of updates increases. On the other hand, by using spatial partitioning with some pruning, we achieve a time complexity that is closer to linear, given that the grid is not too dense.

## What are the next steps?

At the moment, the engine simulates in 2D only. But, with greater processing power access (via university), I have intentions of implementing a 3D engine.
