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

### Simulation controls

`A` (Attractor): Holding `A` activates an attractor in the centre of the window.
`R` (Repeller): Holding `R` activates a repeller in the centre of the window.
`S` (Speed-up): Holding `S` increasingly speeds up each object.
`W` (Slow-down): Holding `W` increasingly slows down each object.
`F` (Slo-mo): Holding `F` slows the simulation down to a near stop.

An interesting thing to note is that these controls can be applied simultaneously. Feel free to experiment with that!

### Simulation parameters

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

Note that lower spawn delay and higher spawn speed can cause extremely rapid movement, which can lead to a crash if values are too extreme.

## How is performance measured?

By using Google Benchmark, I wrote a series of simple benchmarks to analyse the performance of various thread counts, resolvers, and other parameters.

If you'd like to run the benchmarking locally, `cd` into `src`, then run the following commands:

```
git clone https://github.com/google/benchmark.git
cd benchmark
cmake -E make_directory "build"
cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../
cmake --build "build" --config Release
```
The above sets up the [Google Benchmark](https://github.com/google/benchmark/tree/main) dependency locally and sets up the build system.

```
g++ test/benchmark_simulation.cc -std=c++17 -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -lsfml-graphics -lsfml-window -lsfml-system -o test/benchmark_simulation
```
The above then compiles the benchmark file (in case you would like to modify or add benchmarks) into an executable file that can be ran for the actual benchmark analysis.


```
test/benchmark_simulation --benchmark_out=test/benchmark_simulation.csv --benchmark_out_format=csv --benchmark_repetitions=5
```
Finally, the above runs the benchmark executable, which then causes it to output the results into your console and a `.csv` file. To read more about the various options, check out the [Google Benchmark user guide](https://github.com/google/benchmark/blob/main/docs/user_guide.md).

Upon benchmarking, a clear pattern appears. As the brute-force algorithm is of quadratic time complexity with respect to the number of objects, mean operation time unsurprisingly scales quadratically as the number of objects increases. On the other hand, by using spatial partitioning with some pruning, we achieve time complexity that is closer to linear, which is then improved further by a constant through multithreading.

It is important to stress that the time complexity of the spatial partitioning is closer to, but not exactly linear. As the number of objects increases with respect to the size of the window, the average number of objects per cell increases, and this means that objects are more clustered. However, as we increase from 1,000 to 40,000 objects of radius 10 in a 2000 x 2000 window (0.1 to 4 objects per cell), we still retain a loglinear worst case.

As each cell has a side length of the maximum diameter, at 4 objects per cell, we have 4 objects packed into the space available for one -- a scenario unlikely to be simulated -- yet, despite the unrealistic levels of clustering, we sustain a non-quadratic time complexity, illustrating the efficiency of the multithreaded algorithm from average to worst-case scenarios.

## What are the next steps?

At the moment, we can only simulate rigid-body physics. However, I have intentions of simulating soft-body physics later on.
